#pragma once

#include "common/Common.h"
#include "common/Path.h"
#include <unordered_map>
#include <memory>

namespace chisel
{
    using AssetID = uint;
    using AssetTable = std::unordered_map<fs::Path, struct Asset*>;

    struct Asset
    {
        using Path = fs::Path;

        AssetID id = NextID++;

        virtual ~Asset()
        {
            if (AssetPaths.contains(id))
            {
                AssetDB.erase(AssetPaths[id]);
                AssetPaths.erase(id);
            }
        }

        const Path& GetPath() const
        {
            return AssetPaths.contains(id) ? AssetPaths.at(id) : nullPath;
        }

    private:
        friend struct Assets;

        void SetPath(const Path& p)
        {
            AssetDB[p] = this;
            AssetPaths[id] = p;
        }

        static inline AssetTable AssetDB;
        static inline std::unordered_map<AssetID, Path> AssetPaths;
        static inline AssetID NextID = 0;

        static inline const fs::Path nullPath = fs::Path();
    };
}

