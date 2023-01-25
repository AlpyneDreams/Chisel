#pragma once

#include <map>
#include <string>

#include "common/Reflection.h"
#include "imgui/IconsMaterialCommunity.h"

namespace engine::editor
{
    inline std::map<refl::Hash, std::string> ComponentIcons = {
      {refl::TypeHash<struct Transform>, ICON_MC_AXIS_ARROW},
      {refl::TypeHash<struct Camera>, ICON_MC_VIDEO},
    };
}
