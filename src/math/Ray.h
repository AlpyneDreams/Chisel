#pragma once

#include "math/Math.h"
#include "math/AABB.h"
#include "math/Plane.h"

#include <glm/gtx/intersect.hpp>

namespace chisel
{
    // A mathematical ray, defined by a position + direction
    struct Ray
    {
        vec3 origin = vec3(0);
        vec3 direction = vec3(0);
        vec3 invDirection = vec3(0);

        Ray(vec3 origin, vec3 direction)
            : origin(origin)
            , direction(direction)
            , invDirection(1.0f / this->direction)
        {
        }

        vec3 GetPoint(float distance) const
        {
            return origin + distance * direction;
        }

        bool Intersects(const AABB& box) const
        {
            float t1 = (box.min[0] - origin[0]) * invDirection[0];
            float t2 = (box.max[0] - origin[0]) * invDirection[0];

            float tmin = glm::min(t1, t2);
            float tmax = glm::max(t1, t2);

            for (int i = 1; i < 3; i++)
            {
                t1 = (box.min[i] - origin[i]) * invDirection[i];
                t2 = (box.max[i] - origin[i]) * invDirection[i];

                tmin = glm::max(tmin, glm::min(t1, t2));
                tmax = glm::min(tmax, glm::max(t1, t2));
            }

            return tmax > glm::max(tmin, 0.0f);
        }

        bool Intersects(const Plane& plane, float& t) const
        {
            return glm::intersectRayPlane(origin, direction, plane.ProjectPoint(vec3(0.0f)), plane.normal, t);
        }
    };
}
