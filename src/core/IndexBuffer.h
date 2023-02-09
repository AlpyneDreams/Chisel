#pragma once

#include "common/Common.h"
#include "render/Render.h"
#include "GraphicsBuffer.h"

#include <vector>

namespace chisel
{
    struct IndexBuffer final : ElementBuffer
    {
        enum { UInt16, UInt32 } type = UInt32;
        const void* indices;

        IndexBuffer() {}

        IndexBuffer(const uint32* indices, size_t size)
          : ElementBuffer(size / sizeof(uint32)),
            type(UInt32),
            indices(indices)
        {}

        IndexBuffer(const uint16* indices, size_t size)
          : ElementBuffer(size / sizeof(uint16)),
            type(UInt16),
            indices(indices)
        {}

        IndexBuffer(std::vector<uint32>& indices)
          : IndexBuffer(indices.data(), indices.size() * sizeof(uint32))
        {}

        IndexBuffer(std::vector<uint16>& indices)
          : IndexBuffer(indices.data(), indices.size() * sizeof(uint16))
        {}

        inline size_t Stride() const override final {
          return type == UInt32 ? 4 : 2;
        }
    };

}