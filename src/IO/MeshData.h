/**
 * @file MeshData.h
 * @brief 导入结果定义
 * @author hxxcxx
 * @date 2026-04-21
 */
#pragma once

#include "MulanGeo/Engine/Geometry/MeshGeometry.h"

#include <memory>
#include <string>
#include <vector>

namespace MulanGeo::IO {

struct ImportResult {
    std::vector<std::unique_ptr<Engine::MeshGeometry>> meshes;
    std::string sourceFile;
    std::string error;
    bool success = false;
};

} // namespace MulanGeo::IO
