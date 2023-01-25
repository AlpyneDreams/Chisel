#pragma once

#include "common/Common.h"

// Subject to change.
#define ENTT_ID_TYPE    std::uint32_t
#include <entt/entt.hpp>

namespace engine
{
    using EntityID = entt::entity;
    using ComponentID = entt::id_type;
    using Handle = entt::handle;    // TODO: Rename or move into a class?

    inline constexpr EntityID EntityNull = entt::null;

    struct System;
    struct Scene;
    struct Component;
    struct Entity;
    struct Behavior;
}