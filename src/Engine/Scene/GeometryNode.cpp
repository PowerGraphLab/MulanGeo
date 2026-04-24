/**
 * @file GeometryNode.cpp
 * @brief GeometryNode 实现
 * @author hxxcxx
 * @date 2026-04-21
 */

#include "GeometryNode.h"
#include "../RHI/Buffer.h"

namespace MulanGeo::Engine {

// ============================================================
// GPU 缓冲区 lazy 上传
// ============================================================

namespace {
GpuGeometry uploadGeometry(RHIDevice* device, const RenderGeometry& geo) {
    GpuGeometry g;
    g.vertexStride = geo.vertexStride;
    g.vertexCount  = geo.vertexCount;
    g.indexCount   = geo.indexCount;

    if (geo.vertexCount > 0 && !geo.vertexBytes.empty()) {
        g.vertexBuffer = device->createBuffer(BufferDesc::vertex(
            static_cast<uint32_t>(geo.vertexBytes.size()),
            geo.vertexBytes.data(), "GeoVB"));
    }

    if (geo.indexCount > 0 && !geo.indexBytes.empty()) {
        g.indexBuffer = device->createBuffer(BufferDesc::index(
            static_cast<uint32_t>(geo.indexBytes.size()),
            geo.indexBytes.data(), "GeoIB"));
    }

    g.uploaded = true;
    return g;
}
} // anonymous namespace

GpuGeometry* GeometryNode::ensureGpuGeometry(RHIDevice* device) {
    if (!m_gpuGeo.uploaded && hasRenderData()) {
        m_gpuGeo = uploadGeometry(device, m_cachedGeo);
    }
    return m_gpuGeo.isValid() ? &m_gpuGeo : nullptr;
}

GpuGeometry* GeometryNode::ensureGpuEdgeGeometry(RHIDevice* device) {
    if (!m_gpuEdgeGeo.uploaded && hasEdgeData()) {
        m_gpuEdgeGeo = uploadGeometry(device, m_cachedEdgeGeo);
    }
    return m_gpuEdgeGeo.isValid() ? &m_gpuEdgeGeo : nullptr;
}

void GeometryNode::releaseGpuResources() {
    m_gpuGeo = {};
    m_gpuEdgeGeo = {};
}

// ============================================================
// 面管理
// ============================================================

GeometryNode::Face* GeometryNode::findFaceByPickId(uint32_t pickId) {
    for (auto& f : m_faces) {
        if (f.pickId == pickId) return &f;
    }
    return nullptr;
}

const GeometryNode::Face* GeometryNode::findFaceByPickId(uint32_t pickId) const {
    for (const auto& f : m_faces) {
        if (f.pickId == pickId) return &f;
    }
    return nullptr;
}

void GeometryNode::selectFace(uint32_t faceIndex) {
    if (faceIndex < m_faces.size()) {
        m_faces[faceIndex].selected = true;
        setSelected(true);
    }
}

void GeometryNode::deselectFace(uint32_t faceIndex) {
    if (faceIndex < m_faces.size()) {
        m_faces[faceIndex].selected = false;
    }
}

void GeometryNode::toggleFace(uint32_t faceIndex) {
    if (faceIndex < m_faces.size()) {
        m_faces[faceIndex].selected = !m_faces[faceIndex].selected;
    }
}

void GeometryNode::clearFaceSelection() {
    for (auto& f : m_faces) {
        f.selected = false;
    }
    setSelected(false);
}

} // namespace MulanGeo::Engine
