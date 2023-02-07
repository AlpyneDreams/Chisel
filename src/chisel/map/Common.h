#pragma once

#include "core/VertexLayout.h"

namespace chisel
{
    struct Atom {};
    
    namespace Volumes
    {
        enum Volume
        {
            Air, Solid
        };
    }
    
    using Volume = Volumes::Volume;
    
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
