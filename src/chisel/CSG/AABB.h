#pragma once

#include "../CSG/Types.h"

namespace chisel::CSG
{
    struct AABB
    {
        Vector3 min = {};
        Vector3 max = {};

        bool IsDegenerate() const
        {
            return min == max;
        }

        bool Intersects(const AABB& other) const
        {
            return glm::all(glm::lessThanEqual   (min, other.max)) &&
                   glm::all(glm::greaterThanEqual(max, other.min));
        }

        CSG::Vector3 Center() const
        {
            return 0.5f * (min + max);
        }

        static AABB Extend(const AABB& bounds, Vector3 point)
        {
            return AABB
            {
                .min = glm::min(bounds.min, point),
                .max = glm::max(bounds.max, point),
            };
        }
    };
}
