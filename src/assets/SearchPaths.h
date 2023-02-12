#pragma once

#include "platform/Platform.h"
#include "common/Path.h"
#include "common/String.h"

#include <cstdlib>

namespace chisel
{
    inline struct SearchPaths
    {
        using Path = fs::Path;


        Path Resolve(const Path& searchPath)
        {
            static std::string SteamApps = Platform.Windows
                                        ? "C:/Program Files (x86)/Steam/steamapps/common"
                                        : std::string(std::getenv("HOME")) + "/.local/share/Steam/steamapps/common";

            return str::replace(searchPath, "$STEAMAPPS", SteamApps);
        }
    } SearchPaths;
}