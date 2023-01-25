#pragma once

#include <string>

#include "entity/Component.h"

namespace engine
{
    struct Name : Component
    {
        std::string name;
    };
}