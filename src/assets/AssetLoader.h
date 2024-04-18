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
        static std::optional<Buffer> ReadFile(const fs::Path& path);
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

        bool Load(Asset& asset, const fs::Path& path)
        {
            if (!function)
                return false;
            
            auto data = ReadFile(path);
            if (!data)
                return false;
            
            function(asset, *data);
            return true;
        }

    protected:
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
}

