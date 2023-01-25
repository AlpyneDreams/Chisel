#pragma once

#include "entity/Entity.h"

namespace engine
{
    struct Parent : Component
    {
        Entity* parent;
    };
}