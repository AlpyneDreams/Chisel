#pragma once

#include "console/Console.h"
#include "common/String.h"
#include "common/Filesystem.h"
#include "core/Mesh.h"

#include <vector>
#include <map>

namespace chisel
{
    // TODO: Better way to define asset loaders.
    // TODO: Exceptions or Result<T> for FindFile, Load, etc...

    // Override this to support different asset formats.
    // Ext - all uppercase, starts with a dot
    template <class Asset, FixedString Ext>
    Asset* ImportAsset(const fs::Path& path);

    inline struct Assets
    {
        using Path = fs::Path;
        std::vector<Path> searchPaths;
        // TODO: Hashing...
        std::map<Path, void*> loadedAssets;

        Assets()
        {
            // TODO: Load search paths from app info file
            AddSearchPath("core");
        }

    // Asset Loading //

        template <class T, FixedString Ext>
        T* Load(const char* path)
        {
            Path file = FindFile(path);
            if (file.empty())
                return nullptr;

            // Cache hit
            if (loadedAssets.contains(file))
                return (T*)loadedAssets[file];

            // Attempt to load asset for first time
            T* ptr = ImportAsset<T, Ext>(file);
            if (!ptr) {
                Console.Error("[Assets] Failed to import %s asset: %s", Ext.value, path);
                return ptr;
            }

            // Cache loaded asset
            loadedAssets[path] = ptr;

            return ptr;
        }

        // TODO: Dynamic extension
        template <class T>
        T* Load(const char* path) {
            return nullptr;
        }

        // TODO: Map ext -> type
        void* Load(const char* path) {
            return nullptr;
        }

    // Search Paths //

        void AddSearchPath(const char* path)
        {
            Path dir = Path(path);
            if (!fs::exists(dir)) {
                Console.Error("[Assets] Failed to find search path '{}'", path);
                return;
            }
            searchPaths.push_back(dir);
        }

        // Find actual path of asset in search paths
        Path FindFile(const char* path) const
        {
            for (const Path& dir : searchPaths)
            {
                Path fullPath = dir / path;
                if (fs::exists(fullPath))
                    return fullPath;
            }

            Console.Error("[Assets] Can't find file: '{}'", path);
            return Path();
        }

    } Assets;
}
