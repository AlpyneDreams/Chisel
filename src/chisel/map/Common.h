#pragma once

#include "core/VertexLayout.h"
#include "chisel/Selection.h"

#include "Volume.h"

namespace chisel
{
    struct Atom : Selectable
    {
    // Selectable Interface //
        void Delete() final override { Console.Error("Need a reference to the map in here to kill myself."); }
    };

    struct VertexCSG
    {        
        vec3 position;
        vec3 normal;
        vec2 uv;
        
        static inline VertexLayout Layout = VertexLayout {
            VertexAttribute::For<float>(3, VertexAttribute::Position),
            VertexAttribute::For<float>(3, VertexAttribute::Normal, true),
            VertexAttribute::For<float>(2, VertexAttribute::TexCoord),
        };
    };
}
