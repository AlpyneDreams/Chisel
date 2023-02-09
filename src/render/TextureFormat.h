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

            R32F,
            RGBA32F,
            D32F,

            Count,
        };
    }
    using TextureFormat = TextureFormats::TextureFormat;
}