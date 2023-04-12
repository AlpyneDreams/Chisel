#pragma once

#include "assets/Asset.h"
#include "assets/AssetLoader.h"
#include "console/Console.h"
#include "common/Common.h"
#include "common/String.h"
#include "common/Span.h"
#include "common/Filesystem.h"
#include "../submodules/libvpk-plusplus/libvpk++.h"

#include <vector>
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

        template <class T>
        T* Load(const Path& path);

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
        template <typename T>
        void ForEachInDir(const Path& dir, auto func);

        std::vector<Path> searchPaths;
        std::vector<std::unique_ptr<libvpk::VPKSet>> pakFiles;

        static inline AssetTable& AssetDB = Asset::AssetDB;
    } Assets;

    template <class T>
    inline T* Assets::Load(const Path& path)
    {
        // Cache hit
        if (IsLoaded(path)) [[likely]]
            return (T*)AssetDB[path];

        // Lookup file extension
        auto ext = str::toUpper(path.ext());
        auto hash = HashedString(ext);
        if (!AssetLoader<T>::Extensions().contains(hash)) {
            Console.Error("[Assets] No importer for {} file: {}", ext, path);
            return nullptr;
        }

        // Read asset file
        auto data = ReadFile(path);
        if (!data)
            return nullptr;

        // Create the asset
        T* ptr = new T();
        ptr->SetPath(path);

        // Attempt to load asset for first time
        try
        {
            AssetLoader<T>::Extensions()[hash]->Load(*ptr, *data);
        }
        catch (std::exception& err)
        {
            Console.Error("[Assets] Failed to import {} asset: {}", ext, path);
            Console.Error("[Assets] Exception: '{}'", err.what());
            return nullptr;
        }

        return ptr;
    }

    template <typename T>
    inline void Assets::ForEachFile(auto func)
    {
        // Enumerate loose files.
        for (const auto& dir : searchPaths)
        {
            ForEachInDir<T>(dir, func);
        }

        // Enumerate files in paks
        for (const auto& pak : pakFiles)
        {
            auto& files = pak->files();
            for (const auto& file : files)
            {
                auto path = fs::Path(file.first);

                auto ext = str::toUpper(path.ext());
                auto hash = HashedString(ext);

                if (!AssetLoader<T>::Extensions().contains(hash))
                    continue;

                func(path);
            }
        }
    }

    template <typename T>
    inline void Assets::ForEachInDir(const Path& dir, auto func)
    {
        for (auto file : std::filesystem::directory_iterator(dir))
        {
            Path path = file.path();
            bool dir = file.is_directory();
            if (dir)
            {
                ForEachInDir<T>(path, func);
                continue;
            }
            
            auto ext = str::toUpper(path.ext());
            auto hash = HashedString(ext);

            if (!AssetLoader<T>::Extensions().contains(hash))
                continue;
            
            func(path);
        }
    }
}
