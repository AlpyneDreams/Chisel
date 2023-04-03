#pragma once

#include "core/Mesh.h"
#include "assets/Assets.h"

namespace chisel
{
    inline struct Primitives
    {
        static inline Mesh& Cube   = *Assets.Load<Mesh>("models/cube.obj");
        static inline Mesh& Teapot = *Assets.Load<Mesh>("models/teapot.obj");
        static inline Mesh& Plane  = *Assets.Load<Mesh>("models/plane.obj");
        static inline Mesh& Quad   = *Assets.Load<Mesh>("models/quad.obj");
        static inline Mesh Line;

        Primitives()
        {
            auto& g = Line.AddGroup();
            g.vertices.layout.Add<float>(3, VertexAttribute::Position);
            g.vertices.pointer = &lineStart;
            g.vertices.count = 2;
        }

    private:
        vec3 lineStart = Vectors.Zero;
        vec3 lineEnd   = Vectors.Forward;
    } Primitives;
}