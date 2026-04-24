/**
 * @file SceneRenderer.cpp
 * @brief SceneRenderer 实现 — 管理管线资源，录制 RHI 绘制命令
 * @author hxxcxx
 * @date 2026-04-15
 */
#include "SceneRenderer.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace MulanGeo::Engine {

// ============================================================
// GPU UBO 结构 — 与 shader Common.hlsli 对齐
// ============================================================

struct alignas(16) CameraUBO {
    float view[16];
    float projection[16];
    float viewProjection[16];
    float cameraPos[3];
    float _pad0;
};

struct alignas(16) ObjectUBO {
    float world[16];
    // std140: float3x3 占 3×16=48 字节（每列 vec4，末尾 padding）
    float normalMat[12];   // [col0.x .y .z _pad, col1.x .y .z _pad, col2.x .y .z _pad]
    uint32_t pickId;
    float _pad2[3];
};

struct alignas(16) MaterialUBO {
    float baseColor[3];    float _p0;
    float lightDir[3];     float _p1;
    float lightColor[3];   float _p2;
    float ambientColor[3]; float _p3;
    float wireColor[3];    float _p4;
};

// ============================================================
// 构造
// ============================================================

SceneRenderer::SceneRenderer(RHIDevice* device)
    : m_device(device) {}

// ============================================================
// 初始化
// ============================================================

bool SceneRenderer::init() {
    loadShaders();
    if (!m_solidVs || !m_solidFs) return false;

    createPSOs();
    createUBOs();
    return true;
}

void SceneRenderer::cleanup() {
    m_materialBuffer.reset();
    m_objectBuffer.reset();
    m_cameraBuffer.reset();
    m_edgePso.reset();
    m_edgeFs.reset();
    m_edgeVs.reset();
    m_solidPso.reset();
    m_solidFs.reset();
    m_solidVs.reset();
}

void SceneRenderer::finalizePipeline(SwapChain* swapchain) {
    if (m_solidPso && swapchain)
        m_solidPso->finalize(swapchain);
    if (m_edgePso && swapchain)
        m_edgePso->finalize(swapchain);
}

void SceneRenderer::finalizePipeline(RenderTarget* rt) {
    if (m_solidPso && rt)
        m_solidPso->finalize(rt);
    if (m_edgePso && rt)
        m_edgePso->finalize(rt);
}

// ============================================================
// Shader
// ============================================================

void SceneRenderer::loadShaders() {
    auto loadFile = [](const char* path) -> std::vector<uint8_t> {
        FILE* f = nullptr;
#ifdef _WIN32
        fopen_s(&f, path, "rb");
#else
        f = fopen(path, "rb");
#endif
        if (!f) return {};
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        std::vector<uint8_t> data(size);
        fread(data.data(), 1, size, f);
        fclose(f);
        return data;
    };

#ifdef SHADER_DIR
    std::string shaderDir = SHADER_DIR;
#else
    std::string shaderDir = "shaders";
#endif

    const char* ext = ".spv";
    if (m_device->backend() == GraphicsBackend::D3D12)
        ext = ".dxil";
    else if (m_device->backend() == GraphicsBackend::D3D11)
        ext = ".dxbc";

    auto vsPath = shaderDir + "/solid.vert" + ext;
    auto fsPath = shaderDir + "/solid.frag" + ext;
    auto edgeVsPath = shaderDir + "/edge.vert" + ext;
    auto edgeFsPath = shaderDir + "/edge.frag" + ext;

    auto solidVsData = loadFile(vsPath.c_str());
    auto solidFsData = loadFile(fsPath.c_str());
    auto edgeVsData = loadFile(edgeVsPath.c_str());
    auto edgeFsData = loadFile(edgeFsPath.c_str());

    ShaderDesc vsDesc;
    vsDesc.type         = ShaderType::Vertex;
    vsDesc.byteCode     = solidVsData.data();
    vsDesc.byteCodeSize = static_cast<uint32_t>(solidVsData.size());
    m_solidVs = m_device->createShader(vsDesc);

    ShaderDesc fsDesc;
    fsDesc.type         = ShaderType::Pixel;
    fsDesc.byteCode     = solidFsData.data();
    fsDesc.byteCodeSize = static_cast<uint32_t>(solidFsData.size());
    m_solidFs = m_device->createShader(fsDesc);

    if (!edgeVsData.empty() && !edgeFsData.empty()) {
        ShaderDesc edgeVsDesc;
        edgeVsDesc.type         = ShaderType::Vertex;
        edgeVsDesc.byteCode     = edgeVsData.data();
        edgeVsDesc.byteCodeSize = static_cast<uint32_t>(edgeVsData.size());
        m_edgeVs = m_device->createShader(edgeVsDesc);

        ShaderDesc edgeFsDesc;
        edgeFsDesc.type         = ShaderType::Pixel;
        edgeFsDesc.byteCode     = edgeFsData.data();
        edgeFsDesc.byteCodeSize = static_cast<uint32_t>(edgeFsData.size());
        m_edgeFs = m_device->createShader(edgeFsDesc);
    }
}

// ============================================================
// PSO
// ============================================================

void SceneRenderer::createPSOs() {
    m_vertexLayout.begin()
        .add(VertexSemantic::Position,  VertexFormat::Float3)
        .add(VertexSemantic::Normal,    VertexFormat::Float3)
        .add(VertexSemantic::TexCoord0, VertexFormat::Float2);

    GraphicsPipelineDesc solidDesc;
    solidDesc.name                  = "Solid";
    solidDesc.vs                    = m_solidVs.get();
    solidDesc.ps                    = m_solidFs.get();
    solidDesc.vertexLayout          = m_vertexLayout;
    solidDesc.topology              = PrimitiveTopology::TriangleList;
    solidDesc.cullMode              = CullMode::None;
    solidDesc.frontFace             = FrontFace::CounterClockwise;
    solidDesc.fillMode              = FillMode::Solid;
    solidDesc.depthStencil.depthEnable = true;
    solidDesc.depthStencil.depthWrite  = true;
    solidDesc.depthStencil.depthFunc   = CompareFunc::LessEqual;

    using DB = DescriptorBinding;
    solidDesc.descriptorBindings[0] = {0, 1, DB::kStageVertex | DB::kStageFragment};
    solidDesc.descriptorBindings[1] = {1, 1, DB::kStageVertex | DB::kStageFragment};
    solidDesc.descriptorBindings[2] = {2, 1, DB::kStageFragment};
    solidDesc.descriptorBindingCount = 3;

    m_solidPso = m_device->createPipelineState(solidDesc);

    // --- Edge PSO ---
    if (m_edgeVs && m_edgeFs) {
        GraphicsPipelineDesc edgeDesc;
        edgeDesc.name                  = "Edge";
        edgeDesc.vs                    = m_edgeVs.get();
        edgeDesc.ps                    = m_edgeFs.get();
        edgeDesc.vertexLayout          = m_vertexLayout;
        edgeDesc.topology              = PrimitiveTopology::LineList;
        edgeDesc.cullMode              = CullMode::None;
        edgeDesc.frontFace             = FrontFace::CounterClockwise;
        edgeDesc.fillMode              = FillMode::Solid;
        edgeDesc.depthStencil.depthEnable  = true;
        edgeDesc.depthStencil.depthWrite   = false;
        edgeDesc.depthStencil.depthFunc    = CompareFunc::LessEqual;
        // 深度偏移：边线推前避免 z-fighting
        edgeDesc.depthStencil.depthBias        = 1.0f;
        edgeDesc.depthStencil.depthBiasClamp   = 0.0f;
        edgeDesc.depthStencil.slopeScaledDepthBias = 1.5f;

        using DB = DescriptorBinding;
        edgeDesc.descriptorBindings[0] = {0, 1, DB::kStageVertex | DB::kStageFragment};
        edgeDesc.descriptorBindings[1] = {1, 1, DB::kStageVertex | DB::kStageFragment};
        edgeDesc.descriptorBindings[2] = {2, 1, DB::kStageFragment};
        edgeDesc.descriptorBindingCount = 3;

        m_edgePso = m_device->createPipelineState(edgeDesc);
    }
}

void SceneRenderer::createUBOs() {
    auto* dev = m_device;
    m_cameraBuffer = m_device->createBuffer(
        BufferDesc::uniform(sizeof(CameraUBO), "CameraUBO"));

    m_objectBuffer = m_device->createBuffer(
        BufferDesc::uniform(kMaxDrawCalls * sizeof(ObjectUBO), "ObjectUBO_Ring"));

    m_materialBuffer = m_device->createBuffer(
        BufferDesc::uniform(kMaxDrawCalls * sizeof(MaterialUBO), "MaterialUBO_Ring"));

    // 确保默认光照环境有一个方向光
    if (m_lightEnv.lightCount == 0) {
        m_lightEnv.addLight(Light::directional({-0.3, -1.0, -0.4}, {1, 1, 1}, 1.0));
    }

    MaterialUBO mat{};
    mat.baseColor[0] = 0.78f; mat.baseColor[1] = 0.78f; mat.baseColor[2] = 0.78f;
    fillLightingUBO(mat);
    // 边线颜色：深灰
    mat.wireColor[0] = 0.10f; mat.wireColor[1] = 0.10f; mat.wireColor[2] = 0.10f;
    m_materialBuffer->update(0, sizeof(MaterialUBO), &mat);
}

// ============================================================
// Camera UBO 更新
// ============================================================

void SceneRenderer::updateCameraUBO(const Camera& camera) {
    if (!m_cameraBuffer) return;

    CameraUBO ubo{};

    auto view     = camera.viewMatrix();
    auto proj     = camera.projectionMatrix();
    auto clip     = m_device->clipSpaceCorrectionMatrix();
    auto corrProj = clip * proj;
    auto viewProj = corrProj * view;

    auto storeMat4 = [](float* dst, const Mat4& src) {
        const double* p = glm::value_ptr(src);
        for (int i = 0; i < 16; ++i)
            dst[i] = static_cast<float>(p[i]);
    };

    storeMat4(ubo.view,           view);
    storeMat4(ubo.projection,     corrProj);
    storeMat4(ubo.viewProjection, viewProj);

    auto pos = camera.eyePosition();
    ubo.cameraPos[0] = static_cast<float>(pos.x);
    ubo.cameraPos[1] = static_cast<float>(pos.y);
    ubo.cameraPos[2] = static_cast<float>(pos.z);

    m_cameraBuffer->update(0, sizeof(CameraUBO), &ubo);
}

// ============================================================
// PSO finalize（需要在 swapchain/rendertarget 就绪后调用）
// ============================================================

// ============================================================
// 渲染
// ============================================================

void SceneRenderer::render(const RenderQueue& queue, const Camera& camera, CommandList* cmdList,
                           const LightEnvironment& lightEnv) {
    m_stats = {};
    m_lightEnv = lightEnv;
    m_drawCallIndex = 0;

    // 确保至少有一个默认方向光
    if (m_lightEnv.lightCount == 0) {
        m_lightEnv.addLight(Light::directional({-0.3, -1.0, -0.4}));
    }

    if (!m_solidPso) return;

    // 更新 Camera UBO（每帧一次）
    updateCameraUBO(camera);

    Viewport vp{0.0f, 0.0f,
                 static_cast<float>(camera.width()),
                 static_cast<float>(camera.height()),
                 0.0f, 1.0f};
    cmdList->setViewport(vp);

    // --- Pass 1: Solid faces ---
    PipelineState* solidPso = selectPipeline();
    if (solidPso) {
        cmdList->setPipelineState(solidPso);
        for (const auto& item : queue.opaqueItems()) {
            drawItem(item, cmdList, solidPso, false);
        }
    }

    // --- Pass 2: Edge lines (overlay) ---
    PipelineState* edgePso = selectEdgePipeline();
    if (edgePso) {
        cmdList->setPipelineState(edgePso);
        for (const auto& item : queue.edgeItems()) {
            drawItem(item, cmdList, edgePso, true);
        }
    }

    // --- Pass 3: Transparent ---
    PipelineState* transPso = selectPipeline();
    if (transPso) {
        cmdList->setPipelineState(transPso);
        for (const auto& item : queue.transparentItems()) {
            drawItem(item, cmdList, transPso, false);
        }
    }
}

PipelineState* SceneRenderer::selectPipeline() const {
    return m_solidPso.get();
}

PipelineState* SceneRenderer::selectEdgePipeline() const {
    return m_edgePso.get();
}

void SceneRenderer::drawItem(const RenderItem& item, CommandList* cmdList, PipelineState* pso, bool isEdge) {
    if (!item.geometry) return;

    const auto* gpu = item.gpu;
    if (!gpu || !gpu->vertexBuffer) return;

    // 更新 Object UBO（ring buffer offset）
    if (m_objectBuffer && pso) {
        uint32_t ringIndex = m_drawCallIndex % kMaxDrawCalls;
        uint32_t objOffset = ringIndex * sizeof(ObjectUBO);
        uint32_t matOffset = ringIndex * sizeof(MaterialUBO);

        ObjectUBO obj{};
        for (int i = 0; i < 16; ++i)
            obj.world[i] = static_cast<float>(glm::value_ptr(item.worldTransform)[i]);

        Mat3 normalMat3 = glm::transpose(glm::inverse(Mat3(item.worldTransform)));
        for (int col = 0; col < 3; ++col)
            for (int row = 0; row < 3; ++row)
                obj.normalMat[col * 4 + row] = static_cast<float>(normalMat3[col][row]);

        obj.pickId = item.pickId;
        m_objectBuffer->update(objOffset, sizeof(ObjectUBO), &obj);

        // 材质：选中高亮 / 默认
        if (item.selected && m_materialBuffer) {
            MaterialUBO hl{};
            hl.baseColor[0] = 0.3f; hl.baseColor[1] = 0.6f; hl.baseColor[2] = 1.0f;
            fillLightingUBO(hl);
            hl.wireColor[0] = 0.2f; hl.wireColor[1] = 0.5f; hl.wireColor[2] = 1.0f;
            m_materialBuffer->update(matOffset, sizeof(MaterialUBO), &hl);
        } else if (m_materialBuffer) {
            MaterialUBO mat{};
            mat.baseColor[0] = 0.78f; mat.baseColor[1] = 0.78f; mat.baseColor[2] = 0.78f;
            fillLightingUBO(mat);
            mat.wireColor[0] = 0.10f; mat.wireColor[1] = 0.10f; mat.wireColor[2] = 0.10f;
            m_materialBuffer->update(matOffset, sizeof(MaterialUBO), &mat);
        }

        RHIDevice::UniformBufferBind uboBinds[] = {
            { 0, m_cameraBuffer.get(),   0, sizeof(CameraUBO) },
            { 1, m_objectBuffer.get(),   objOffset, sizeof(ObjectUBO) },
            { 2, m_materialBuffer.get(), matOffset, sizeof(MaterialUBO) },
        };
        m_device->bindUniformBuffers(cmdList, pso, uboBinds, 3);
    }

    cmdList->setVertexBuffer(0, gpu->vertexBuffer.get());

    if (gpu->indexBuffer && gpu->indexCount > 0) {
        cmdList->setIndexBuffer(gpu->indexBuffer.get());
        cmdList->drawIndexed(DrawIndexedAttribs{
            .indexCount = gpu->indexCount,
        });
        if (isEdge) {
            m_stats.lines += gpu->indexCount / 2;
        } else {
            m_stats.triangles += gpu->indexCount / 3;
        }
    } else {
        cmdList->draw(DrawAttribs{
            .vertexCount = gpu->vertexCount,
        });
        if (isEdge) {
            m_stats.lines += gpu->vertexCount / 2;
        } else {
            m_stats.triangles += gpu->vertexCount / 3;
        }
    }

    ++m_stats.drawCalls;
    ++m_stats.items;
    ++m_drawCallIndex;
}

// ============================================================
// 光照 UBO 填充辅助
// ============================================================

void SceneRenderer::fillLightingUBO(MaterialUBO& mat) {
    if (auto* dl = m_lightEnv.primaryDirectional()) {
        mat.lightDir[0] = static_cast<float>(dl->direction.x);
        mat.lightDir[1] = static_cast<float>(dl->direction.y);
        mat.lightDir[2] = static_cast<float>(dl->direction.z);
        float intensity = static_cast<float>(dl->intensity) * 3.5f;
        mat.lightColor[0] = static_cast<float>(dl->color.x) * intensity;
        mat.lightColor[1] = static_cast<float>(dl->color.y) * intensity;
        mat.lightColor[2] = static_cast<float>(dl->color.z) * intensity;
    }
    mat.ambientColor[0] = static_cast<float>(m_lightEnv.ambientColor.x * m_lightEnv.ambientIntensity);
    mat.ambientColor[1] = static_cast<float>(m_lightEnv.ambientColor.y * m_lightEnv.ambientIntensity);
    mat.ambientColor[2] = static_cast<float>(m_lightEnv.ambientColor.z * m_lightEnv.ambientIntensity);
}

} // namespace MulanGeo::Engine
