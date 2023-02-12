#pragma once

#include "platform/Platform.h"
#include "common/Path.h"
#include "common/String.h"

namespace chisel
{
    inline struct SearchPaths
    {
        using Path = fs::Path;


        Path Resolve(const Path& searchPath)
        {
            static constexpr auto SteamApps = Platform.Windows
                                            ? "C:/Program Files (x86)/Steam/steamapps/common"
                                            : "$HOME/.local/share/Steam/steamapps/common";

            return str::replace(searchPath, "$STEAMAPPS", SteamApps);
        }
    } SearchPaths;
}