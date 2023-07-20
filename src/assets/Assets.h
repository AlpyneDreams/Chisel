#pragma once

#include "assets/Asset.h"
#include "assets/AssetLoader.h"
#include "console/Console.h"
#include "common/Common.h"
#include "common/String.h"
#include "common/Span.h"
#include "common/Filesystem.h"
#include "../submodules/libvpk-plusplus/libvpk++.h"

#include <list>
#include <unordered_map>

namespace chisel
{
    // TODO: Exceptions or Result<T> for FindFile, Load, etc...

    inline struct Assets
    {
        using Path = fs::Path;

        Assets();
        ~Assets();

    // Asset Loading //

        bool IsLoaded(const Path& path);

        template <typename T>
        Rc<T> Load(const Path& path);

        std::optional<Buffer> ReadFile(const Path& path);
        std::optional<Buffer> ReadLooseFile(const Path& path);
        std::optional<Buffer> ReadPakFile(const Path& path);

    // Search Paths //

        void AddSearchPath(const Path& p);
        void AddPakFile(const Path& p);

    // File Enumeration //

        template <typename T>
        void ForEachFile(auto func);

    private:
        std::list<Path> searchPaths;
        std::list<std::unique_ptr<libvpk::VPKSet>> pakFiles;
    } Assets;

    template <typename T>
    inline Rc<T> Assets::Load(const Path& path)
    {
        // Cache hit
        if (IsLoaded(path)) [[likely]]
            return Rc<T>(static_cast<T*>(T::AssetDB[path]));

        // Lookup file extension
        auto* loader = AssetLoader<T>::ForExtension(path.ext());
        if (!loader) {
            Console.Error("[Assets] No importer for {} file: {}", path.ext(), path);
            return nullptr;
        }

        // Read asset file
        auto data = ReadFile(path);
        if (!data)
            return nullptr;

        // Create the asset
        Rc<T> asset = new T(path);

        // Attempt to load asset for first time
        try
        {
            loader->Load(*asset.ptr(), *data);
        }
        catch (std::exception& err)
        {
            Console.Error("[Assets] Failed to import {} asset: {}", path.ext(), path);
            Console.Error("[Assets] Exception: '{}'", err.what());
            return nullptr;
        }

        return asset;
    }

    template <typename T>
    inline void Assets::ForEachFile(auto func)
    {
        // Enumerate loose files.
        for (const auto& dir : searchPaths)
        {
            for (auto file : std::filesystem::recursive_directory_iterator(dir))
            {
                Path path = file.path();
                if (file.is_directory())
                    continue;
                
                auto ext = str::toUpper(path.ext());
                auto hash = HashedString(ext);

                if (!AssetLoader<T>::ForExtension(path.ext()))
                    continue;
                
                func(path);
            }
        }

        // Enumerate files in paks
        for (const auto& pak : pakFiles)
        {
            auto& files = pak->files();
            for (const auto& file : files)
            {
                auto path = fs::Path(file.first);

                if (!AssetLoader<T>::ForExtension(path.ext()))
                    continue;

                func(path);
            }
        }
    }
}
