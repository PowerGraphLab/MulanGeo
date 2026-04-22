/**
 * @file OCCTShapeGeometry.cpp
 * @brief OCCTShapeGeometry 实现
 * @author hxxcxx
 * @date 2026-04-22
 */
#include "OCCTShapeGeometry.h"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepBndLib.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <TopAbs.hxx>

#include <algorithm>

namespace MulanGeo::Document {

OCCTShapeGeometry::OCCTShapeGeometry(TopoDS_Shape shape)
    : m_shape(std::move(shape))
{}

const Engine::MeshGeometry* OCCTShapeGeometry::displayMesh() const {
    std::lock_guard lock(m_cacheMutex);
    if (!m_meshGenerated) {
        m_cachedMesh = triangulate();
        m_meshGenerated = true;
    }
    return m_cachedMesh.get();
}

Engine::AABB OCCTShapeGeometry::boundingBox() const {
    Bnd_Box box;
    BRepBndLib::Add(m_shape, box);
    if (box.IsVoid()) return Engine::AABB::empty();

    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    Engine::AABB result;
    result.min = {xmin, ymin, zmin};
    result.max = {xmax, ymax, zmax};
    return result;
}

std::unique_ptr<Engine::MeshGeometry> OCCTShapeGeometry::triangulate() const {
    // 计算线性偏差
    Bnd_Box box;
    BRepBndLib::Add(m_shape, box);
    if (box.IsVoid()) return nullptr;

    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    double dx = xmax - xmin;
    double dy = ymax - ymin;
    double dz = zmax - zmin;
    double maxDim = std::max({dx, dy, dz});
    double linearDeflection = maxDim * 0.001; // 0.1% of max dimension

    // 复制 shape 用于三角化（BRepMesh 会修改 shape）
    TopoDS_Shape shapeToMesh = m_shape;
    BRepMesh_IncrementalMesh mesher(shapeToMesh, linearDeflection, false, 0.5, true);
    mesher.Perform();
    if (!mesher.IsDone()) return nullptr;

    auto mesh = std::make_unique<Engine::MeshGeometry>();

    for (TopExp_Explorer faceExp(shapeToMesh, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
        const auto& face = TopoDS::Face(faceExp.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull()) continue;

        const gp_Trsf& trsf = loc.Transformation();
        uint32_t baseIndex = static_cast<uint32_t>(mesh->vertexCount());

        int nbNodes = tri->NbNodes();
        for (int i = 1; i <= nbNodes; i++) {
            gp_Pnt p = tri->Node(i).Transformed(trsf);
            gp_Dir n(0, 0, 1);
            if (tri->HasNormals()) {
                n = tri->Normal(i).Transformed(trsf);
            }
            mesh->vertices.push_back(static_cast<float>(p.X()));
            mesh->vertices.push_back(static_cast<float>(p.Y()));
            mesh->vertices.push_back(static_cast<float>(p.Z()));
            mesh->vertices.push_back(static_cast<float>(n.X()));
            mesh->vertices.push_back(static_cast<float>(n.Y()));
            mesh->vertices.push_back(static_cast<float>(n.Z()));
            mesh->vertices.push_back(0.0f); // texCoord u
            mesh->vertices.push_back(0.0f); // texCoord v
        }

        int nbTris = tri->NbTriangles();
        for (int i = 1; i <= nbTris; i++) {
            int n0, n1, n2;
            tri->Triangle(i).Get(n0, n1, n2);
            mesh->indices.push_back(baseIndex + static_cast<uint32_t>(n0 - 1));
            mesh->indices.push_back(baseIndex + static_cast<uint32_t>(n1 - 1));
            mesh->indices.push_back(baseIndex + static_cast<uint32_t>(n2 - 1));
        }
    }

    if (!mesh->empty()) {
        mesh->computeBounds();
    }

    return mesh;
}

} // namespace MulanGeo::Document
