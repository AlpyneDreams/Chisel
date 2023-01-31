#pragma once

#include "math/Math.h"

namespace chisel
{
    struct AABB
    {
        vec3 min = {};
        vec3 max = {};

        bool IsDegenerate() const {
            return min == max;
        }

        bool Intersects(const AABB& other) const
        {
            return glm::all(glm::lessThanEqual   (min, other.max)) &&
                   glm::all(glm::greaterThanEqual(max, other.min));
        }

        vec3 Center() const {
            return 0.5f * (min + max);
        }

        static AABB Extend(const AABB& bounds, vec3 point)
        {
            return AABB
            {
                .min = glm::min(bounds.min, point),
                .max = glm::max(bounds.max, point),
            };
        }
        
        static AABB Extend(const AABB& bounds, const AABB aabb) {
            return Extend(Extend(bounds, aabb.min), aabb.max);
        }
        
        AABB Extend(vec3 point) const {
            return Extend(*this, point);
        }
        
        AABB Extend(const AABB aabb) const {
            return Extend(*this, aabb);
        }
    };
}
