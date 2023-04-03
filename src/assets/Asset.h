#pragma once

#include "common/Path.h"
#include <unordered_map>

namespace chisel
{
    
    using AssetTable = std::unordered_map<fs::Path, struct Asset*>;

    struct Asset
    {
        using Path = fs::Path;

        virtual ~Asset()
        {
            if (path)
                AssetDB.erase(path);
        }

        const Path& GetPath() const
        {
            return path;
        }

    private:
        friend struct Assets;

        Path path;

        void SetPath(const Path& p)
        {
            path = p;
            AssetDB[path] = this;
        }

        static inline AssetTable AssetDB;
    };
}

