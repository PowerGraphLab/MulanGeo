/**
 * @file MeshDocument.cpp
 * @brief MeshDocument 实现
 * @author hxxcxx
 * @date 2026-04-21
 */
#include "MulanGeo/IO/MeshDocument.h"

#include <filesystem>

namespace MulanGeo::IO {

std::unique_ptr<MeshDocument> MeshDocument::fromImportResult(
    ImportResult result, std::string filePath)
{
    auto doc = std::make_unique<MeshDocument>();
    doc->m_filePath    = std::move(filePath);
    doc->m_displayName = std::filesystem::path(doc->m_filePath).filename().string();
    doc->m_geometries  = std::move(result.meshes);
    return doc;
}

std::string MeshDocument::summary() const {
    size_t verts = 0, tris = 0;
    for (const auto& m : m_geometries) {
        verts += m->vertexCount();
        tris  += m->triangleCount();
    }
    return std::to_string(m_geometries.size()) + " parts | "
         + std::to_string(verts) + " verts | "
         + std::to_string(tris) + " tris";
}

} // namespace MulanGeo::IO
