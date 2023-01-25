#pragma once

#include "Common.h"
#include "common/Reflection.h"

namespace engine
{
    // Base class for all data or logic components.
    // Provides runtime reflection.
    struct Component : rain::Reflect
    {
    };

    // Attribute to require other components. Must be publically inherited.
    template <class... Components>
    struct RequireComponents
    {
    protected:
        friend struct Entity;
        static constexpr void AddRequiredComponents(Handle& h) {
            ((void)h.get_or_emplace<Components>(), ...);
        }
    };

    // RequireComponent: Alias for RequireComponents
    template <class... Components>
    using RequireComponent = RequireComponents<Components...>;

}