#pragma once

#include "../CSG/Types.h"

namespace chisel::CSG
{
    // A mathematical plane, defined by a normal + offset
    struct Plane
    {
        Plane()
            {}

        Plane(Vector3 normal, Unit offset)
            : normal(normal), offset(offset) {}

        Plane(Vector3 point, Vector3 normal)
            : Plane(normal, -glm::dot(point, normal)) {}

        Plane(Vector3 a, Vector3 b, Vector3 c)
            : Plane(a, NormalFromPoints(a, b, c)) {}

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

            return Plane{ transformedOrigin, transformedNormal };
        }

        static CSG::Vector3 NormalFromPoints(const CSG::Vector3& a, const CSG::Vector3& b, const CSG::Vector3& c)
        {
            const CSG::Vector3 v1 = a - b;
            const CSG::Vector3 v2 = c - b;
            return glm::normalize(glm::cross(v1, v2));
        }
    };
}
