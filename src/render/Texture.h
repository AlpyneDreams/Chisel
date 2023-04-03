#pragma once

#include "common/Common.h"
#include "common/Filesystem.h"
#include "assets/Asset.h"
#include "core/GraphicsBuffer.h"
#include "render/TextureFormat.h"
#include "console/Console.h"

#include <cstdlib>
#include <span>

namespace chisel
{
    struct Texture : GraphicsBuffer, Asset
    {
        using Format = render::TextureFormat;

        fs::Path path;
        uint16_t width;
        uint16_t height;
        uint16_t depth;
        Format format = Format::None;

        bool uploaded = false;  // Uploaded to GPU memory

        std::unique_ptr<uint8_t[]> owned_data;
        std::span<const uint8_t> data;

        Texture() {}
        Texture(uint16_t width, uint16_t height, uint16_t depth, Format format, std::span<const uint8_t> data)
          : GraphicsBuffer()
          , width(width)
          , height(height)
          , depth(depth)
          , format(format)
          , data(data)
        {}
        Texture(uint16_t width, uint16_t height, uint16_t depth, Format format, std::unique_ptr<uint8_t[]> owned_data, size_t size)
          : GraphicsBuffer()
          , width(width)
          , height(height)
          , depth(depth)
          , format(format)
          , owned_data(std::move(owned_data))
          , data(this->owned_data.get(), size)
        {}

        bool Valid() const {
            return !data.empty();
        }

        size_t Size() const {
            return data.size();
        }

        // Call after uploading to free from main memory
        void Free()
        {
            if (owned_data) {
                owned_data = nullptr;
                data = std::span<const uint8_t>();
            }
        }
    };
}