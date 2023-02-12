#pragma once

#include "console/Console.h"
#include "common/String.h"
#include "common/Filesystem.h"
#include "core/Mesh.h"
#include "../submodules/libvpk-plusplus/libvpk++.h"

#include <vector>
#include <map>

namespace chisel
{
    // TODO: Better way to define asset loaders.
    // TODO: Exceptions or Result<T> for FindFile, Load, etc...

    // Override this to support different asset formats.
    // Ext - all uppercase, starts with a dot
    template <class Asset, FixedString Ext>
    Asset* ImportAsset(std::string_view path, std::vector<uint8_t> data);

    inline struct Assets
    {
        using Path = fs::Path;

        std::vector<Path> searchPaths;
        std::vector<std::unique_ptr<libvpk::VPKSet>> pakFiles;
        std::unordered_map<Path, void*> loadedAssets;

        Assets()
        {
            // TODO: Load search paths from app info file
            AddSearchPath("core");
            AddPakFile("/home/joshua/.local/share/Steam/steamapps/common/Half-Life 2/hl2/hl2_textures");
            AddPakFile("/home/joshua/.local/share/Steam/steamapps/common/Half-Life 2/hl1/hl1_pak");
        }

    // Asset Loading //

        bool IsLoaded(const Path& path)
        {
            return loadedAssets.contains(path);
        }

        template <class T, FixedString Ext>
        T* Load(const Path& path)
        {
            // Cache hit
            if (IsLoaded(path))
                return (T*)loadedAssets[path];

            auto data = ReadFile(path);
            if (!data)
                return nullptr;

            // Attempt to load asset for first time
            T* ptr = ImportAsset<T, Ext>(path, std::move(*data));
            if (!ptr) {
                Console.Error("[Assets] Failed to import {} asset: {}", Ext.value, path);
                return nullptr;
            }

            // Cache loaded asset
            loadedAssets[path] = ptr;

            return ptr;
        }

        std::optional<std::vector<uint8_t>> ReadFile(const Path& path)
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

        std::optional<std::vector<uint8_t>> ReadLooseFile(const Path& path)
        {
            for (const auto& dir : searchPaths)
            {
                Path fullPath = dir / path;
                if (fs::exists(fullPath))
                    return fs::readFileBinary(fullPath);
            }
            return std::nullopt;
        }

        std::optional<std::vector<uint8_t>> ReadPakFile(const Path& path)
        {
            std::string lower_string = str::toLower(path);

            for (const auto& pak : pakFiles)
            {
                auto file = pak->file(lower_string);
                if (!file)
                    continue;

                auto stream = libvpk::VPKFileStream(*file);

                std::vector<uint8_t> data;
                data.resize(file->length());
                stream.read((char*)data.data(), file->length());

                return data;
            }
            return std::nullopt;
        }

        // TODO: Dynamic extension
        template <class T>
        T* Load(const Path& path) {
            return nullptr;
        }

        // TODO: Map ext -> type
        void* Load(const Path& path) {
            return nullptr;
        }

    // Search Paths //

        void AddSearchPath(const Path& path)
        {
            if (!fs::exists(path)) {
                return Console.Error("[Assets] Failed to find search path '{}'", path);
            }
            searchPaths.push_back(path);
        }

        void AddPakFile(const Path& path)
        {
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

    } Assets;
}
