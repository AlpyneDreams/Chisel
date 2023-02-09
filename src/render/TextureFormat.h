#pragma once

#include "common/Common.h"

namespace chisel::render
{
    // TODO: All texture formats...
    namespace TextureFormats {
        enum TextureFormat : uint32_t
        {
            None = 0,

            R8,
            RG8,
            RGBA8,
            BGRA8,
            B5G6R5,
            R5G6B5,

            BC1,
            BC2,
            BC3,
            BC4,
            BC5,
            BC6H,
            BC7,

            R32F,
            RG32F,
            RGBA32F,
            D32F,

            Count,
        };
    }
    using TextureFormat = TextureFormats::TextureFormat;
}