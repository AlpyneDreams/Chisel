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
        std::vector<std::string> searchPaths;
        std::vector<std::unique_ptr<libvpk::VPKSet>> pakFiles;
        // TODO: Hashing...
        std::unordered_map<std::string, void*> loadedAssets;

        Assets()
        {
            // TODO: Load search paths from app info file
            AddSearchPath("core");
            AddPakFile("/home/joshua/.local/share/Steam/steamapps/common/Half-Life 2/hl1/hl1_pak");
        }

    // Asset Loading //

        template <class T, FixedString Ext>
        T* Load(std::string_view path)
        {
            // Ugh.
            auto pathStr = std::string(path);

            // Cache hit
            if (loadedAssets.contains(pathStr))
                return (T*)loadedAssets[pathStr];

            auto data = ReadFile(pathStr);
            if (!data)
                return nullptr;

            // Attempt to load asset for first time
            T* ptr = ImportAsset<T, Ext>(path, std::move(*data));
            if (!ptr) {
                Console.Error("[Assets] Failed to import %s asset: %s", Ext.value, path);
                return nullptr;
            }

            // Cache loaded asset
            loadedAssets[pathStr] = ptr;

            return ptr;
        }

        std::optional<std::vector<uint8_t>> ReadFile(const std::string& path)
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

        std::optional<std::vector<uint8_t>> ReadLooseFile(const std::string& path)
        {
            for (const auto& dir : searchPaths)
            {
                fs::Path fullPath = fs::Path(dir) / fs::Path(path);
                if (fs::exists(fullPath))
                    return fs::readFileBinary(fullPath);
            }
            return std::nullopt;
        }

        std::optional<std::vector<uint8_t>> ReadPakFile(const std::string& path)
        {
            for (const auto& pak : pakFiles)
            {
                auto file = pak->file(path);
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
        T* Load(std::string_view path) {
            return nullptr;
        }

        // TODO: Map ext -> type
        void* Load(std::string_view path) {
            return nullptr;
        }

    // Search Paths //

        void AddSearchPath(std::string_view path)
        {
            if (!fs::exists(fs::Path(path))) {
                Console.Error("[Assets] Failed to find search path '{}'", path);
                return;
            }
            searchPaths.push_back(std::string(path));
        }

        void AddPakFile(std::string_view path)
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
