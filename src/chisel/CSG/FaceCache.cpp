#include "../CSG/FaceCache.h"
#include "../CSG/Vertex.h"
#include "../CSG/Plane.h"

namespace chisel::CSG
{
    FaceCache::FaceCache(const std::vector<Plane>& planes)
    {
        size_t count = planes.size();
        m_faces.reserve(count);
        for (const auto& plane : planes)
            m_faces.push_back(Face(&plane));

        for (size_t i = 0; i < count - 2; i++)
        {
            for (size_t j = i + 1; j < count - 1; j++)
            {
                for (size_t k = j + 1; k < count; k++)
                {
                    std::array<Face*, 3> trio {{ &m_faces[i], &m_faces[j], &m_faces[k] }};
                    if (std::optional<Vertex> vertex = Face::CreateVertex(trio))
                    {
                        if (CalculateVertexRelation(*vertex) != BrushVertexRelations::Outside)
                        {
                            for (Face* face : trio)
                                face->vertices.push_back(*vertex);

                            m_bounds = m_bounds
                                ? AABB::Extend(*m_bounds, vertex->position)
                                : AABB{ vertex->position, vertex->position };
                        }
                    }
                }
            }
        }

        for (auto& face : m_faces)
        {
            face.OrderVertices();
            face.FixWinding();
        }
    }

    BrushVertexRelation FaceCache::CalculateVertexRelation(const Vertex& vertex) const
    {
        BrushVertexRelation type = BrushVertexRelations::Inside;
        for (const auto& face : m_faces)
        {
            switch (face.CalculateVertexRelation(vertex))
            {
                case FaceVertexRelations::Front:   return BrushVertexRelations::Outside;
                case FaceVertexRelations::Aligned: return BrushVertexRelations::Aligned;
                default: break;
            }
        }
        return type;
    }
}
