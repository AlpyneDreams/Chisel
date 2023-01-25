#pragma once

#include "common/Common.h"
#include "common/Filesystem.h"
#include "core/GraphicsBuffer.h"
#include "render/TextureFormat.h"
#include "console/Console.h"

#include <cstdlib>


namespace engine
{
    struct Texture : GraphicsBuffer
    {
        using Format = render::TextureFormat;

        fs::Path path;
        uint width, height;
        uint channels = 4;
        uint bitsPerChannel = 8;
        Format format = Format::None;

        bool uploaded;  // Uploaded to GPU memory
        bool hasData;   // Has data in main memory
        byte* data;

        Texture() {}
        Texture(uint width, uint height, byte* data, uint n, uint bits = 8)
          : GraphicsBuffer(width * height),
            width(width), height(height), channels(n), bitsPerChannel(bits),
            format(render::PickTextureFormat(n, bits)), uploaded(false), hasData(true), data(data)
        {}

        size_t Stride() const {
            return channels * bitsPerChannel / 8;
        }

        // Call after uploading to free from main memory
        void Free()
        {
            if (!hasData)
                return;
            hasData = false;
            std::free(data);
        }
    };
}