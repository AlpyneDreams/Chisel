#pragma once

namespace chisel
{
    enum class TransformType {
        Translate, Rotate, Scale, Universal, Bounds
    };

    enum class ClipType : int {
        Front, Back, KeepBoth
    };

    enum class SelectMode {
        Groups, Objects, Solids, Faces, Edges, Vertices
    };

    enum class PrimitiveType {
        Block, Quad, Stairs, Arch, Cylinder, Sphere, Cone, Torus, Wedge
    };
}
