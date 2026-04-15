/*
 * 渲染几何数据 — GPU 无关的通用可绘制数据描述
 *
 * RenderGeometry: 顶点/索引的原始字节 + 布局描述 + 拓扑类型
 * RenderItem:     一次绘制所需的全部数据（几何 + 变换 + 元数据）
 * RenderQueue:    RenderItem 集合，由 SceneAdapter 填充，Renderer 消费
 *
 * 不依赖任何 IO 类型，不依赖任何具体数据来源。
 * 三角网格、线框、2D 图纸、文字 quad、点云等统一表示。
 */

#pragma once

#include "../Math/Mat4.h"
#include "../RHI/VertexLayout.h"
#include "../RHI/PipelineState.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace MulanGeo::Engine {

// ============================================================
// 可绘制几何数据 — 原始字节 + 元信息
// ============================================================

struct RenderGeometry {
    std::span<const std::byte> vertexBytes;    // 顶点数据（视图，不拥有）
    std::span<const std::byte> indexBytes;     // 索引数据（视图，不拥有，可为空）
    uint32_t           vertexCount  = 0;
    uint32_t           indexCount   = 0;
    uint32_t           vertexStride = 0;       // 单个顶点字节数
    VertexLayout       layout;                 // 怎么解释 vertexBytes
    PrimitiveTopology  topology = PrimitiveTopology::TriangleList;

    // 便捷：顶点数据总字节数
    uint32_t vertexDataSize() const { return vertexCount * vertexStride; }
    // 便捷：索引数据总字节数
    uint32_t indexDataSize() const {
        return static_cast<uint32_t>(indexBytes.size());
    }
    // 是否有索引
    bool hasIndex() const { return indexCount > 0 && !indexBytes.empty(); }
};

// ============================================================
// 绘制项 — 一次 draw call 的全部数据
// ============================================================

struct RenderItem {
    const RenderGeometry* geometry       = nullptr;
    Mat4                  worldTransform = Mat4::identity();
    uint32_t              pickId         = 0;
    // 后续可扩展: materialIndex, sortKey, renderPassFlags...
};

// ============================================================
// 渲染队列 — 由上层填充，Renderer 消费
// ============================================================

class RenderQueue {
public:
    void add(const RenderItem& item) { m_items.push_back(item); }
    void clear() { m_items.clear(); }
    void reserve(size_t n) { m_items.reserve(n); }

    size_t size() const { return m_items.size(); }
    bool   empty() const { return m_items.empty(); }

    // 遍历
    const RenderItem* data() const { return m_items.data(); }
    std::span<const RenderItem> items() const { return m_items; }

    const RenderItem& operator[](size_t i) const { return m_items[i]; }

private:
    std::vector<RenderItem> m_items;
};

} // namespace MulanGeo::Engine
