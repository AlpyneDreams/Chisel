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

        vec3 Dimensions() const {
            return glm::abs(max - min);
        }

        // Compute the matrix to transform a 1x1 cube at the origin to this box
        mat4x4 ComputeMatrix() const {
            return glm::scale(glm::translate(mat4x4(1), Center()), Dimensions());
        }

    // Extending //

        static [[nodiscard]] AABB Extend(const AABB& bounds, vec3 point)
        {
            return AABB
            {
                .min = glm::min(bounds.min, point),
                .max = glm::max(bounds.max, point),
            };
        }
        
        static [[nodiscard]] AABB Extend(const AABB& bounds, const AABB aabb) {
            return Extend(Extend(bounds, aabb.min), aabb.max);
        }
        
        [[nodiscard]] AABB Extend(vec3 point) const {
            return Extend(*this, point);
        }
        
        [[nodiscard]] AABB Extend(const AABB aabb) const {
            return Extend(*this, aabb);
        }
    };

    inline AABB operator*(const mat4x4& mtx, const AABB& bounds)
    {
        // min and max are points so w=1
        return AABB { mtx * vec4(bounds.min, 1), mtx * vec4(bounds.max, 1) };
    }

    inline AABB operator*(const mat3x3& mtx, const AABB& bounds)
    {
        return AABB { mtx * bounds.min, mtx * bounds.max };
    }
}
