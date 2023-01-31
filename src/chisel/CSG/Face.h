#pragma once

#include "../CSG/Types.h"
#include "../CSG/Vertex.h"
#include "../CSG/Fragment.h"

namespace chisel::CSG
{
    struct Face
    {
        FaceVertexRelation CalculateVertexRelation(const Vertex& vertex) const;

        void OrderVertices();
        void FixWinding();

        void RebuildFragments(VolumeID voidVolume, const Brush& brush);

        static std::optional<Vertex> CreateVertex(std::array<Face*, 3> faces);

        const Side* side;
        std::vector<Vertex> vertices;
        std::vector<Fragment> fragments;
    };
}
