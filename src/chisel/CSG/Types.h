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
#include "math/Plane.h"
#include <glm/gtc/matrix_access.hpp>

namespace chisel::CSG
{
    using Plane = chisel::Plane;
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

    using math::CloseEnough;
}
