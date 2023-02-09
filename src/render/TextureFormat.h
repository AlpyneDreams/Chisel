#pragma once

#include "common/Common.h"

namespace chisel::render
{
    // TODO: All texture formats...
    enum class TextureFormat
    {
        None = 0,

        R8,
        RG8,
        RGBA8,

        R32F,
        RGBA32F,
        D32F,
    };
}