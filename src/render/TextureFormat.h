#pragma once

#include "common/Common.h"

namespace engine::render
{
    // TODO: All texture formats...
    enum class TextureFormat
    {
        None = 0,

        R8,
        RG8,
        RGB8,
        RGBA8,

        R32F,
        RGBA32F,
        D32F,
    };

    constexpr TextureFormat PickTextureFormat(uint channels, uint bitsPerChannel = 8)
    {
        using enum TextureFormat;

        switch (bitsPerChannel)
        {
            case 8:
                switch(channels)
                {
                    case 1: return R8;
                    case 2: return RG8;
                    case 3: return RGB8;
                    case 4: return RGBA8;
                    default: break;
                }
            default:
                return None;
        }
    }
}