#pragma once

#include "common/Common.h"
#include "render/Render.h"

#include <cstddef>

namespace chisel
{
    // Base class for VertexBuffer and IndexBuffer
    struct GraphicsBuffer
    {
        // GPU buffer handle
        void* handle = nullptr;

        GraphicsBuffer() {}

        virtual ~GraphicsBuffer() {}

        // Total size of this buffer
        virtual size_t Size() const = 0;
    };

    struct ElementBuffer : GraphicsBuffer
    {
        // Number of items in this buffer
        uint count = 0;

        ElementBuffer() {}

        ElementBuffer(uint count) : count(count) {}

        // Number of bytes per item in this buffer
        virtual size_t Stride() const = 0;

        // Total size of this buffer
        virtual size_t Size() const {
            return count * this->Stride();
        }
    };
}