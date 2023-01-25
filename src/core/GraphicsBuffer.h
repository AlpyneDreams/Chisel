#pragma once

#include "common/Common.h"
#include "render/Render.h"

#include <cstddef>

namespace engine
{
    // Base class for VertexBuffer and IndexBuffer
    struct GraphicsBuffer
    {
        // Number of items in this buffer
        uint count = 0;

        // GPU buffer handle
        render::Handle* handle = nullptr;

        GraphicsBuffer() {}

        virtual ~GraphicsBuffer() {}

        GraphicsBuffer(uint count) : count(count) {}

        // True if buffer contains any data
        operator bool() const { return count > 0; }

        // Number of bytes per item in this buffer
        virtual size_t Stride() const = 0;

        // Total size of this buffer
        inline size_t Size() const {
            return count * Stride();
        }
    };

}