#pragma once

#include "assets/Asset.h"
#include "assets/AssetLoader.h"
#include "assets/SearchPaths.h"
#include "console/Console.h"
#include "common/Common.h"
#include "common/String.h"
#include "common/Span.h"
#include "common/Filesystem.h"
#include "core/Mesh.h"
#include "../submodules/libvpk-plusplus/libvpk++.h"

#include <vector>

namespace chisel
{
    // TODO: Exceptions or Result<T> for FindFile, Load, etc...

    inline struct Assets
    {
        using Path = fs::Path;
        using Buffer = std::vector<byte>;

        std::vector<Path> searchPaths;
        std::vector<std::unique_ptr<libvpk::VPKSet>> pakFiles;

        static inline AssetTable& AssetDB = Asset::AssetDB;

        Assets()
        {
            // TODO: Load search paths from app info file
            AddSearchPath("core");
            AddPakFile("$STEAMAPPS/Half-Life 2/hl2/hl2_textures");
            AddPakFile("$STEAMAPPS/Half-Life 2/hl2/hl2_misc");
            AddPakFile("$STEAMAPPS/Half-Life 2/hl1/hl1_pak");
            AddPakFile("$STEAMAPPS/Counter-Strike Source/cstrike/cstrike_pak");
        }

        ~Assets()
        {
            // Delete all remaining assets on the heap
            std::vector<Asset*> assets;
            for (auto& [path, asset] : AssetDB)
                assets.push_back(asset);
            for (Asset* asset : assets)
                delete asset;
            AssetDB.clear();
        }

    // Asset Loading //

        bool IsLoaded(const Path& path)
        {
            return AssetDB.contains(path);
        }

        template <class T>
        T* Load(const Path& path)
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

        std::optional<Buffer> ReadFile(const Path& path)
        {
            auto loose_data = ReadLooseFile(path);
            if (loose_data)
                return loose_data;

            auto vpk_data = ReadPakFile(path);
            if (vpk_data)
                return vpk_data;

            Console.Error("[Assets] Can't find file: '{}'", path);
            return std::nullopt;
        }

        std::optional<Buffer> ReadLooseFile(const Path& path)
        {
            for (const auto& dir : searchPaths)
            {
                Path fullPath = dir / path;
                if (fs::exists(fullPath))
                    return fs::readFile(fullPath);
            }
            return std::nullopt;
        }

        std::optional<Buffer> ReadPakFile(const Path& path)
        {
            std::string lower_string = str::toLower(path);
            lower_string = str::replace(lower_string, "\\", "/");

            for (const auto& pak : pakFiles)
            {
                auto file = pak->file(lower_string);
                if (!file)
                    continue;

                auto stream = libvpk::VPKFileStream(*file);

                Buffer data;
                data.resize(file->length());
                stream.read((char*)data.data(), file->length());

                return data;
            }
            return std::nullopt;
        }

    // Search Paths //

        void AddSearchPath(const Path& p)
        {
            Path path = SearchPaths.Resolve(p);
            if (!fs::exists(path)) {
                return Console.Error("[Assets] Failed to find search path '{}'", path);
            }
            searchPaths.push_back(path);
        }

        void AddPakFile(const Path& p)
        {
            Path path = SearchPaths.Resolve(p);
            try
            {
                auto pak = std::make_unique<libvpk::VPKSet>(path);
                pakFiles.emplace_back(std::move(pak));
            }
            catch (const std::exception& e)
            {
                Console.Error("[Assets] Failed to load pak file '{}': {}", path, e.what());
            }
        }

        template <typename T>
        void ForEachFile(auto func)
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

    private:
        template <typename T>
        void ForEachInDir(const Path& dir, auto func)
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

    } Assets;
}
