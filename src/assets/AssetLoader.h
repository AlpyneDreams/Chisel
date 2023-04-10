#pragma once

#include "common/Span.h"
#include "common/String.h"
#include "console/Console.h"
#include "common/Hash.h"
#include <unordered_map>
#include <span>

namespace chisel
{

    template <class Asset, auto... Ext>
    struct AssetLoader;

    template <class Asset>
    struct AssetLoader<Asset>
    {
        using AssetLoadFn = void(Asset&, const Buffer&);

        AssetLoader(AssetLoadFn* fn) : function(fn) {}

        virtual void Load(Asset& asset, const Buffer& buffer)
        {
            if (function)
                function(asset, buffer);
        }

    private:
        AssetLoadFn* function = nullptr;

    protected:
        friend struct Assets;
        static inline std::unordered_map<Hash, AssetLoader<Asset>*>& Extensions() {
            static std::unordered_map<Hash, AssetLoader<Asset>*> map;
            return map;
        }
    };

    template <class Asset, FixedString Ext>
    struct AssetLoader<Asset, Ext> : AssetLoader<Asset>
    {
        AssetLoader(auto fn) : AssetLoader<Asset>(fn)
        {
            auto upper = str::toUpper(Ext);
            auto hash  = HashString(upper.data(), upper.size());
            AssetLoader<Asset>::Extensions().insert({hash, this});
        }
    };
}

