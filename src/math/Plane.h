#pragma once

#include "math/Math.h"

namespace chisel
{
    // A mathematical plane, defined by a normal + offset
    struct Plane
    {
        vec3  normal = vec3(0);
        float offset = 0;

        Plane() = default;

        Plane(vec3 normal, float offset)
            : normal(normal), offset(offset) {}

        Plane(vec3 point, vec3 normal)
            : Plane(normal, -glm::dot(point, normal)) {}

        Plane(vec3 a, vec3 b, vec3 c)
            : Plane(a, NormalFromPoints(a, b, c)) {}


        float SignedDistance(const vec3& point) const
        {
            return glm::dot(normal, point) + offset;
        }

        vec3 ProjectPoint(const vec3& point) const
        {
            return point - SignedDistance(point) * normal;
        }

        Plane Transformed(const mat4x4& matrix) const
        {
            const vec3 transformedOrigin = vec3{ matrix * vec4{ ProjectPoint(vec3(0.0, 0.0, 0.0)), 1.0 } };
            const vec3 transformedNormal = glm::normalize(vec3{ glm::transpose(glm::inverse(matrix)) * vec4{ normal, 0.0 } });

            return Plane{ transformedOrigin, transformedNormal };
        }

        static vec3 NormalFromPoints(const vec3& a, const vec3& b, const vec3& c)
        {
            const vec3 v1 = a - b;
            const vec3 v2 = c - b;
            return glm::normalize(glm::cross(v1, v2));
        }

        float Dist() const
        {
            return -offset;
        }
    };
}
