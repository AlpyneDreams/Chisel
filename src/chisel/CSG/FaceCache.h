#pragma once

#include "math/AABB.h"

#include "../CSG/Types.h"
#include "../CSG/Face.h"
#include "../CSG/Relation.h"

namespace chisel::CSG
{
    class FaceCache
    {
    public:
        FaceCache(const std::vector<Plane>& planes);

        std::vector<Face>& GetFaces() { return m_faces; }
        const std::vector<Face>& GetFaces() const { return m_faces; }

        std::optional<AABB> GetBounds() const { return m_bounds; }
    private:
        BrushVertexRelation CalculateVertexRelation(const Vertex& vertex) const;

        std::vector<Face> m_faces;
        std::optional<AABB> m_bounds;
    };
}
