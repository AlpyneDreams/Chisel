#pragma once

#include "../CSG/Types.h"

#include "../CSG/Relation.h"
#include "../CSG/Vertex.h"
#include "../CSG/Volume.h"

namespace chisel::CSG
{
    struct FragmentDirData
    {
        VolumeID volume = DefaultVolume;
        const Brush* brush = nullptr;
    };

    using TriangleIndices = std::array<uint32_t, 3>;

    struct Fragment
    {
        Face* face = nullptr;
        FragmentDirData front;
        FragmentDirData back;
        std::vector<Vertex> vertices;

        std::optional<FragmentFaceRelation> relation;

        FragmentFaceRelation CalculateFaceRelation(const Face& otherFace) const;

        std::vector<TriangleIndices> Triangulate() const;
    };
}