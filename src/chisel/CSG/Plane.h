#pragma once

#include "../CSG/Types.h"

namespace chisel::CSG
{
    struct Plane
    {
        Plane()
            {}

        Plane(Vector3 normal, Unit offset)
            : normal(normal), offset(offset) {}

        Plane(Vector3 point, Vector3 normal)
            : Plane(normal, -glm::dot(point, normal)) {}

        Vector3 normal = {};
        Unit    offset = {};

        float SignedDistance(const Vector3& point) const
        {
            return glm::dot(normal, point) + offset;
        }

        Vector3 ProjectPoint(const Vector3& point) const
        {
            return point - SignedDistance(point) * normal;
        }

        Plane Transformed(const Matrix4& matrix) const
        {
            const Vector3 transformedOrigin = Vector3{ matrix * Vector4{ ProjectPoint(Vector3(0.0, 0.0, 0.0)), 1.0 } };
            const Vector3 transformedNormal = glm::normalize(Vector3{ glm::transpose(glm::inverse(matrix)) * Vector4{ normal, 0.0 } });

            printf("TRANSFORMED: %g %g %g - %g %g %g\n",
                transformedOrigin.x, transformedOrigin.y, transformedOrigin.z,
                transformedNormal.x, transformedNormal.y, transformedNormal.z);

            return Plane{ transformedOrigin, transformedNormal };
        }
    };
}
