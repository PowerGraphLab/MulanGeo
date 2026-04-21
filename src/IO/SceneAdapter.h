/**
 * @file SceneAdapter.h
 * @brief 场景适配器，IO层与Engine渲染层的桥接
 * @author hxxcxx
 * @date 2026-04-21
 */

#pragma once

#include "MulanGeo/Engine/Scene/Scene.h"
#include "MulanGeo/Engine/Scene/Camera.h"
#include "MulanGeo/Engine/Scene/GeometryNode.h"
#include "MulanGeo/Engine/Render/RenderGeometry.h"

namespace MulanGeo::IO {

class SceneAdapter {
public:
    // 从场景树收集可见的 RenderItem 到队列
    // scene:    场景树
    // camera:   用于视锥体裁剪
    // queue:    输出的渲染队列
    void collect(Engine::Scene& scene, const Engine::Camera& camera,
                 Engine::RenderQueue& queue)
    {
        queue.clear();
        m_geometries.clear();
        auto frustum = camera.frustum();

        scene.traverseVisible([&](Engine::SceneNode& node) {
            auto* geoNode = node.as<Engine::GeometryNode>();
            if (!geoNode) return;

            auto* mesh = geoNode->mesh();
            if (!mesh || mesh->empty()) return;

            // 视锥体裁剪
            const auto& bounds = node.boundingBox();
            if (!bounds.isEmpty() && !frustum.intersects(bounds)) return;

            // MeshGeometry → RenderGeometry（零拷贝）
            m_geometries.push_back(mesh->asRenderGeometry());

            Engine::RenderItem item;
            item.geometry       = &m_geometries.back();
            item.worldTransform = node.worldTransform();
            item.pickId         = node.pickId();

            queue.add(item);
        });
    }

private:
    std::vector<Engine::RenderGeometry> m_geometries;
};

} // namespace MulanGeo::IO
