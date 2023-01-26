#include "../CSG/Fragment.h"
#include "../CSG/Face.h"
#include "../CSG/Plane.h"

namespace chisel::CSG
{
    FragmentFaceRelation Fragment::CalculateFaceRelation(const Face& otherFace) const
    {
        std::array<uint32_t, 3> counts = {};

        for (const auto& vertex : vertices)
            counts[otherFace.CalculateVertexRelation(vertex)]++;

        if (counts[FaceVertexRelations::Front] > 0 &&
            counts[FaceVertexRelations::Back] > 0)
            return FragmentFaceRelations::Split;

        if (counts[FaceVertexRelations::Front] == 0 &&
            counts[FaceVertexRelations::Back] == 0)
        {
            return glm::dot(otherFace.plane->normal, this->face->plane->normal) < 0
                ? FragmentFaceRelations::ReverseAligned
                : FragmentFaceRelations::Aligned;
        }

        return counts[FaceVertexRelations::Back] > 0
            ? FragmentFaceRelations::Inside
            : FragmentFaceRelations::Outside;
    }

    std::vector<TriangleIndices> Fragment::Triangulate() const
    {
        // Very naiive rn. Should maybe use delaugney.
        // Replace :(
        uint32_t vertex_count = uint32_t(vertices.size());
        std::vector<uint32_t> indices;
        std::vector<TriangleIndices> tris;
        indices.resize(vertex_count);
        for (uint32_t i=0; i<vertex_count; ++i)
            indices[i] = i;
        for (;;) {
            uint32_t n = uint32_t(indices.size());
            if (n < 3)
                break;
            if (n == 3) {
                tris.push_back(TriangleIndices{indices[0], indices[1], indices[2]});
                break;
            }
            tris.push_back(TriangleIndices{indices[0], indices[1], indices[2]});
            tris.push_back(TriangleIndices{indices[2], indices[3], indices[4 % n]});
            indices.erase(indices.begin() + 3);
            indices.erase(indices.begin() + 1);
        }
        return tris;
    }
}
