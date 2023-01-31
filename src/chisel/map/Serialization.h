#pragma once

#include "../CSG/CSGTree.h"
#include "../CSG/Brush.h"
#include "../CSG/Side.h"

#include <glaze/glaze.hpp>

// This file isn't used right now.

template <>
struct glz::meta<chisel::CSG::CSGTree>
{
    using T = chisel::CSG::CSGTree;
    static constexpr auto value = glz::object(
        "void_volume", &T::m_void,
        "brushes", &T::m_brushes
    );
};

template <>
struct glz::meta<chisel::CSG::Brush>
{
    using T = chisel::CSG::Brush;
    static constexpr auto value = glz::object(
        "object_id", &T::m_objectId,
        "order", &T::m_order,
        "sides", &T::m_sides
    );
};

template <>
struct glz::meta<chisel::CSG::Side>
{
    using T = chisel::CSG::Side;
    static constexpr auto value = glz::object(
        "plane", &T::plane
    );
};

template <>
struct glz::meta<chisel::CSG::Plane>
{
    using T = chisel::CSG::Plane;
    static constexpr auto value = glz::object(
        "normal", &T::normal,
        "offset", &T::offset
    );
};

