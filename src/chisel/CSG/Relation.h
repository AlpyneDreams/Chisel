#pragma once

#include "../CSG/Types.h"

namespace chisel::CSG
{
    namespace FaceVertexRelations
    {
        enum FaceVertexRelation : uint32_t
        {
            Front,
            Back,
            Aligned,

            Count,
        };
    }
    using FaceVertexRelation = FaceVertexRelations::FaceVertexRelation;

    namespace BrushVertexRelations
    {
        enum BrushVertexRelation : uint32_t
        {
            Outside,
            Inside,
            Aligned,

            Count,
        };
    }
    using BrushVertexRelation = BrushVertexRelations::BrushVertexRelation;

    namespace FragmentFaceRelations
    {
        enum FragmentFaceRelation : uint32_t
        {
            Outside,
            Inside,
            Aligned,
            ReverseAligned,
            Split,

            Count,
        };
    }
    using FragmentFaceRelation = FragmentFaceRelations::FragmentFaceRelation;
}
