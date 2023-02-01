#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <list>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "math/Math.h"
#include <glm/gtc/matrix_access.hpp>

namespace chisel::CSG
{
    struct Plane;
    class Brush;
    struct Face;
    struct Fragment;
    struct Vertex;
    class CSGTree;
    class FaceCache;
    struct Side;
    struct UserdataProvider;

    using Unit    = float;
    using Vector3 = glm::vec<3, Unit, glm::defaultp>;
    using Vector4 = glm::vec<4, Unit, glm::defaultp>;
    using Matrix3 = glm::mat<3, 3, Unit, glm::defaultp>;
    using Matrix4 = glm::mat<4, 4, Unit, glm::defaultp>;

    using ObjectID = uint32_t;
    using Order = uint32_t;

    static constexpr Unit EQUAL_EPSILON = 0.001f;

    inline bool CloseEnough(Unit a, Unit b, Unit epsilon = EQUAL_EPSILON)
    {
        return std::abs(a - b) <= epsilon;
    }

    inline bool CloseEnough(const Vector3& a, const Vector3& b, Unit epsilon = EQUAL_EPSILON)
    {
        return std::abs(a.x - b.x) <= epsilon &&
               std::abs(a.y - b.y) <= epsilon &&
               std::abs(a.z - b.z) <= epsilon;
    }
}
