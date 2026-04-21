/**
 * @file GeometryNode.h
 * @brief 几何场景节点，持有 MeshGeometry 引用
 * @author hxxcxx
 * @date 2026-04-21
 */

#pragma once

#include "SceneNode.h"
#include "../Geometry/MeshGeometry.h"

namespace MulanGeo::Engine {

// Engine 层节点类型标签
constexpr NodeType GeometryNodeType = 1;

// ============================================================
// 几何节点 — 场景中可渲染的三角网格
// ============================================================

class GeometryNode final : public SceneNode {
public:
    static constexpr NodeType kType = GeometryNodeType;

    explicit GeometryNode(std::string name = {}, uint32_t pickId = 0)
        : SceneNode(kType, std::move(name), pickId) {}

    MeshGeometry* mesh() const { return m_mesh; }
    void setMesh(MeshGeometry* mesh) { m_mesh = mesh; }

private:
    MeshGeometry* m_mesh = nullptr;  // 不拥有，指向 Document 中的数据
};

} // namespace MulanGeo::Engine
