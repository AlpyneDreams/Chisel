#pragma once

#include "math/Ray.h"

namespace chisel
{
    class Solid;
    struct Face;

    struct RayHit
    {
        const Solid* brush;
        const Face* face;
        float t;
    };
}