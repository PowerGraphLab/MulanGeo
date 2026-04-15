/*
 * 场景 — 管理场景节点树
 *
 * 提供根节点、添加/删除/查找节点、
 * 世界变换更新、可见性遍历等操作。
 */

#pragma once

#include "SceneNode.h"

#include <cstdint>
#include <functional>
#include <string_view>

namespace MulanGeo::Engine {

class Scene {
public:
    Scene() {
        m_root = std::make_unique<SceneNode>(SceneNode::Type::Base, "__root__");
    }

    // --- 根节点 ---

    SceneNode* root() { return m_root.get(); }
    const SceneNode* root() const { return m_root.get(); }

    // --- 便捷：添加节点 ---

    // 添加到根节点（SceneNode 基类，装配体用）
    SceneNode* addNode(std::string name, uint32_t pickId = 0) {
        return addNode(root(), std::move(name), pickId);
    }

    // 添加到指定父节点（SceneNode 基类）
    SceneNode* addNode(SceneNode* parent, std::string name, uint32_t pickId = 0) {
        auto child = std::make_unique<SceneNode>(SceneNode::Type::Base, std::move(name), pickId);
        return parent->addChild(std::move(child));
    }

    // 添加网格节点到根
    MeshNode* addMeshNode(std::string name, uint32_t pickId = 0) {
        return addMeshNode(root(), std::move(name), pickId);
    }

    // 添加网格节点到指定父节点
    MeshNode* addMeshNode(SceneNode* parent, std::string name, uint32_t pickId = 0) {
        auto child = std::make_unique<MeshNode>(std::move(name), pickId);
        return static_cast<MeshNode*>(parent->addChild(std::move(child)));
    }

    // --- 查找 ---

    // 按 pickId 查找（深度优先）
    SceneNode* findByPickId(uint32_t pickId) {
        return findByPickId(root(), pickId);
    }

    // 按名称查找（深度优先，返回第一个匹配）
    SceneNode* findByName(std::string_view name) {
        return findByName(root(), name);
    }

    // --- 变换更新 ---

    // 更新整棵树的世界变换（从根开始级联）
    void updateWorldTransforms() {
        updateWorldTransform(root(), Mat4::identity());
    }

    // --- 遍历 ---

    // 遍历所有实际可见的 MeshNode
    using VisibleMeshCallback = std::function<void(
        const MeshNode* node,
        const MeshNode::MeshPart& mesh,
        const Mat4& worldTransform)>;

    void forEachVisibleMesh(const VisibleMeshCallback& callback) const {
        traverseVisibleMesh(root(), callback);
    }

    // --- 统计 ---

    size_t nodeCount() const {
        size_t count = 0;
        countNodes(root(), count);
        return count;
    }

    size_t meshNodeCount() const {
        size_t count = 0;
        countMeshNodes(root(), count);
        return count;
    }

    // --- 清空 ---

    void clear() {
        m_root = std::make_unique<SceneNode>(SceneNode::Type::Base, "__root__");
    }

private:
    // 深度优先查找
    static SceneNode* findByPickId(SceneNode* node, uint32_t pickId) {
        if (!node) return nullptr;
        if (node->pickId() == pickId) return node;
        for (auto& child : node->children()) {
            auto* found = findByPickId(child.get(), pickId);
            if (found) return found;
        }
        return nullptr;
    }

    static SceneNode* findByName(SceneNode* node, std::string_view name) {
        if (!node) return nullptr;
        if (node->name() == name) return node;
        for (auto& child : node->children()) {
            auto* found = findByName(child.get(), name);
            if (found) return found;
        }
        return nullptr;
    }

    // 级联更新世界变换
    static void updateWorldTransform(SceneNode* node, const Mat4& parentWorld) {
        if (!node) return;
        auto newWorld = parentWorld * node->localTransform();
        node->m_worldTransform = newWorld;  // 需要友元或改为 public setter
        node->m_worldDirty = false;
        for (auto& child : node->children()) {
            updateWorldTransform(child.get(), newWorld);
        }
    }

    // 遍历可见网格
    static void traverseVisibleMesh(const SceneNode* node,
                                     const VisibleMeshCallback& cb) {
        if (!node || !node->isEffectivelyVisible()) return;
        if (node->isMeshNode()) {
            auto* meshNode = node->as<MeshNode>();
            if (meshNode && meshNode->meshData()) {
                cb(meshNode, *meshNode->meshData(), meshNode->worldTransform());
            }
        }
        for (auto& child : node->children()) {
            traverseVisibleMesh(child.get(), cb);
        }
    }

    static void countNodes(const SceneNode* node, size_t& count) {
        if (!node) return;
        ++count;
        for (auto& child : node->children()) {
            countNodes(child.get(), count);
        }
    }

    static void countMeshNodes(const SceneNode* node, size_t& count) {
        if (!node) return;
        if (node->isMeshNode()) ++count;
        for (auto& child : node->children()) {
            countMeshNodes(child.get(), count);
        }
    }

    std::unique_ptr<SceneNode> m_root;
};

} // namespace MulanGeo::Engine
