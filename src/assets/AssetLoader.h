#pragma once

#include "common/Span.h"
#include "common/String.h"
#include "console/Console.h"
#include "common/Hash.h"
#include "common/Path.h"
#include <unordered_map>
#include <span>

namespace chisel
{
    struct BaseAssetLoader
    {
        static std::optional<Buffer> ReadFile(const fs::Path& path, bool complain = true);
    };

    template <class Asset>
    struct AssetLoader : BaseAssetLoader
    {
        using AssetLoadFn = void(Asset&, const Buffer&);

        AssetLoader(const char* ext, AssetLoadFn* fn) : function(fn)
        {
            if (!ext)
                return;

            if (ext[0] != '.')
            {
                std::string dotext = std::string(".") + ext;
                AssetLoader<Asset>::Extensions().insert({ HashStringLower(dotext), this });
            }
            else
            {
                AssetLoader<Asset>::Extensions().insert({ HashStringLower(ext), this });
            }
        }

        virtual bool Load(Asset& asset, const fs::Path& path)
        {
            if (!function)
                return false;
            
            auto data = BaseAssetLoader::ReadFile(path);
            if (!data)
                return false;
            
            function(asset, *data);
            return true;
        }

    protected:
        AssetLoader() {}

        AssetLoadFn* function = nullptr;

    public:
        static AssetLoader* ForExtension(std::string_view ext)
        {
            if (!ext.size())
                return nullptr;
            auto hash = HashStringLower(ext);
            return AssetLoader::Extensions().contains(hash)
                 ? AssetLoader::Extensions()[hash]
                 : nullptr;
        }

    protected:
        friend struct Assets;
        static inline std::unordered_map<Hash, AssetLoader<Asset>*>& Extensions()
        {
            static std::unordered_map<Hash, AssetLoader<Asset>*> map;
            return map;
        }
    };

    template <class Asset>
    struct MultiFileAssetLoader final : AssetLoader<Asset>
    {
        using MultiAssetLoadFn = void(Asset&, const std::span<Buffer>& buffers);

        MultiFileAssetLoader(std::initializer_list<const char*> exts, MultiAssetLoadFn* fn) : multiFunction(fn), extensions(exts)
        {
            if (!extensions.size())
                return;

            for (auto ext : extensions)
            {
                if (!ext)
                    continue;

                if (ext[0] != '.')
                {
                    std::string dotext = std::string(".") + ext;
                    AssetLoader<Asset>::Extensions().insert({ HashStringLower(dotext), this });
                }
                else
                {
                    AssetLoader<Asset>::Extensions().insert({ HashStringLower(ext), this });
                }
            }
        }

        virtual bool Load(Asset& asset, const fs::Path& path) override
        {
            if (!multiFunction)
                return false;

            bool foundAny = false;
            std::vector<Buffer> buffers;
            for (auto ext : extensions)
            {
                fs::Path file = path;
                file.setExt(ext);
                auto data = BaseAssetLoader::ReadFile(file, false);
                if (data)
                    foundAny = true;
                buffers.push_back(data ? *data : Buffer());
            }

            if (!foundAny)
                return false;
            
            multiFunction(asset, buffers);
            return true;
        }

    protected:
        MultiAssetLoadFn* multiFunction = nullptr;
        std::vector<const char*> extensions;
    };
}

