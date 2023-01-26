#include "../CSG/Vertex.h"
#include "../CSG/Edge.h"

namespace chisel::CSG
{
    std::optional<Edge> Vertex::FindEdge(const Vertex& vertex0, const Vertex& vertex1)
    {
        // An edge exists if the two vertices share two faces.
        std::array<Face*, 3> faces;
        auto it = std::set_intersection(
            std::begin(vertex0.faces), std::end(vertex0.faces),
            std::begin(vertex1.faces), std::end(vertex1.faces),
            faces.begin()
        );

        // Use std::distance?
        if (it != faces.begin() + 2)
            return std::nullopt;

        return Edge{{faces[0], faces[1]}};
    }
}
