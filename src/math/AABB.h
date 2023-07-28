#pragma once

#include "math/Math.h"
#include <array>

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

        [[nodiscard]] static AABB Extend(const AABB& bounds, vec3 point)
        {
            return AABB
            {
                .min = glm::min(bounds.min, point),
                .max = glm::max(bounds.max, point),
            };
        }
        
        [[nodiscard]] static AABB Extend(const AABB& bounds, const AABB aabb) {
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

    inline auto AABBToCorners(const AABB& aabb)
    {
        std::array<vec3, 8> corners =
        {{
            vec3( aabb.min.x, aabb.min.y, aabb.min.z ),
            vec3( aabb.min.x, aabb.min.y, aabb.max.z ),
            vec3( aabb.min.x, aabb.max.y, aabb.min.z ),
            vec3( aabb.min.x, aabb.max.y, aabb.max.z ),
            vec3( aabb.max.x, aabb.min.y, aabb.min.z ),
            vec3( aabb.max.x, aabb.min.y, aabb.max.z ),
            vec3( aabb.max.x, aabb.max.y, aabb.min.z ),
            vec3( aabb.max.x, aabb.max.y, aabb.max.z ),
        }};

        return corners;
    }
}
