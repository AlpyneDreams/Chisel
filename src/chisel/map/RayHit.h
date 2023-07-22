#pragma once

#include "math/Ray.h"

namespace chisel
{
    struct Solid;
    struct Face;

    struct RayHit
    {
        const Solid* brush;
        const Face* face;
        float t;
    };
}