#pragma once

namespace chisel
{
    enum class Tool {
        Select, Translate, Rotate, Scale, Universal, Bounds,
        Entity, Block
    };

    enum class SelectMode {
        Groups, Objects, Solids
    };

    enum class PrimitiveType {
        Block, Quad, Stairs, Arch, Cylinder, Sphere, Cone, Torus, Wedge
    };
}
