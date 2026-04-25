/**
 * @file GeometryNode.h
 * @brief 几何场景节点，持有缓存的渲染几何数据
 * @author hxxcxx
 * @date 2026-04-21
 */

#pragma once

#include "SceneNode.h"
#include "../Render/RenderGeometry.h"
#include "../RHI/Device.h"

#include <cstdint>

namespace MulanGeo::Engine {

// ============================================================
// 几何节点 — 场景中可渲染的几何体
//
// 持有从 MeshGeometry 缓存的 RenderGeometry，
// 构建（SceneBuilder）时一次性生成，每帧直接使用无需重建。
// ============================================================

class GeometryNode final : public SceneNode {
public:
    static constexpr MulanGeo::NodeType kType = MulanGeo::NodeType::Geometry;

    explicit GeometryNode(std::string name = {}, uint32_t pickId = 0)
        : SceneNode(kType, std::move(name), pickId) {}

    // --- 缓存的渲染几何（由 SceneBuilder 一次性设置）---

    /// 设置缓存的 RenderGeometry（从 MeshGeometry::asRenderGeometry 生成）
    void setCachedRenderGeometry(const RenderGeometry& geo) { m_cachedGeo = geo; }

    /// 获取缓存的渲染几何（每帧直接使用，不重建）
    const RenderGeometry& cachedRenderGeometry() const { return m_cachedGeo; }

    /// 设置缓存的边线 RenderGeometry
    void setCachedEdgeGeometry(const RenderGeometry& geo) { m_cachedEdgeGeo = geo; }

    /// 获取缓存的边线渲染几何
    const RenderGeometry& cachedEdgeGeometry() const { return m_cachedEdgeGeo; }

    // --- GPU 缓冲区（首次渲染时 lazy 上传）---

    /// 确保面几何已上传到 GPU，返回 GPU 缓冲区指针
    GpuGeometry* ensureGpuGeometry(RHIDevice* device);

    /// 确保边线几何已上传到 GPU，返回 GPU 缓冲区指针
    GpuGeometry* ensureGpuEdgeGeometry(RHIDevice* device);

    /// 释放 GPU 缓冲区（场景切换时调用）
    void releaseGpuResources();

    /// 是否有可渲染面数据
    bool hasRenderData() const { return m_cachedGeo.vertexCount > 0; }

    /// 是否有可渲染边线数据
    bool hasEdgeData() const { return m_cachedEdgeGeo.vertexCount > 0; }

    /// 材质索引（用于 RenderQueue 排序合并）
    uint16_t materialIndex() const { return m_materialIndex; }
    void setMaterialIndex(uint16_t idx) { m_materialIndex = idx; }

private:
    RenderGeometry   m_cachedGeo;
    RenderGeometry   m_cachedEdgeGeo;
    GpuGeometry      m_gpuGeo;
    GpuGeometry      m_gpuEdgeGeo;
    uint16_t         m_materialIndex = 0xFFFF;
};

} // namespace MulanGeo::Engine
