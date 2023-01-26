#pragma once

#include "../CSG/Types.h"
#include "../CSG/Edge.h"

namespace chisel::CSG
{
    struct Vertex
    {
        static std::optional<Edge> FindEdge(const Vertex& vertex0, const Vertex& vertex1);

        std::array<Face*, 3> faces = {};
        Vector3 position = {};
    };
}
