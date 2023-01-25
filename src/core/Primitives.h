#pragma once

#include "core/Mesh.h"
#include "assets/Assets.h"

namespace chisel
{
    inline struct Primitives
    {
        static inline Mesh Cube   = *Assets.Load<Mesh, ".OBJ">("models/cube.obj");
        static inline Mesh Teapot = *Assets.Load<Mesh, ".OBJ">("models/teapot.obj");
        static inline Mesh Plane  = *Assets.Load<Mesh, ".OBJ">("models/plane.obj");
        static inline Mesh Quad   = *Assets.Load<Mesh, ".OBJ">("models/quad.obj");
    } Primitives;
}