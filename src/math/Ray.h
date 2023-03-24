#pragma once

#include "math/Math.h"

namespace chisel
{
    // A mathematical ray, defined by a position + direction
    struct Ray
    {
        vec3 origin = vec3(0);
        vec3 direction = vec3(0);

        Ray(vec3 origin, vec3 direction)
            : origin(origin), direction(glm::normalize(direction)) {}

        vec3 GetPoint(float distance) const
        {
            return origin + distance * direction;
        }
    };
}
