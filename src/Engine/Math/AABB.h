/*
 * 轴对齐包围盒 — 视锥体裁剪、碰撞检测
 *
 * CAD 查看器用于：快速可见性剔除、点击拾取范围检测。
 */

#pragma once

#include "Vec3.h"
#include "Mat4.h"

namespace MulanGeo::Engine {

#include <limits>

struct AABB {
    Vec3 min = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    Vec3 max = {std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};

    static AABB empty() { return {}; }

    static AABB fromCenterExtents(const Vec3& center, const Vec3& extents) {
        return {center - extents, center + extents};
    }

    // --- 操作 ---

    bool isEmpty() const {
        return min.x > max.x || min.y > max.y || min.z > max.z;
    }

    void reset() {
        min = {FLT_MAX, FLT_MAX, FLT_MAX};
        max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    }

    // 扩展以包含点
    void expand(const Vec3& p) {
        min = Vec3::min(min, p);
        max = Vec3::max(max, p);
    }

    // 扩展以包含另一个 AABB
    void expand(const AABB& b) {
        if (b.isEmpty()) return;
        min = Vec3::min(min, b.min);
        max = Vec3::max(max, b.max);
    }

    // --- 查询 ---

    Vec3 center() const { return (min + max) * 0.5f; }
    Vec3 extents() const { return (max - min) * 0.5f; }
    Vec3 size() const { return max - min; }

    bool contains(const Vec3& p) const {
        return p.x >= min.x && p.x <= max.x
            && p.y >= min.y && p.y <= max.y
            && p.z >= min.z && p.z <= max.z;
    }

    bool intersects(const AABB& b) const {
        return (min.x <= b.max.x && max.x >= b.min.x)
            && (min.y <= b.max.y && max.y >= b.min.y)
            && (min.z <= b.max.z && max.z >= b.min.z);
    }

    // 用矩阵变换 AABB，返回新的轴对齐包围盒
    AABB transformed(const Mat4& m) const {
        if (isEmpty()) return empty();

        AABB result;
        // 变换 8 个角点，取新的包围盒
        for (int i = 0; i < 8; ++i) {
            Vec3 corner(
                (i & 1) ? max.x : min.x,
                (i & 2) ? max.y : min.y,
                (i & 4) ? max.z : min.z
            );
            result.expand(m.transformPoint(corner));
        }
        return result;
    }
};

} // namespace MulanGeo::Engine
