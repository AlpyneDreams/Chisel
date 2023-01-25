#pragma once

#include "common/Common.h"
#include "render/Render.h"
#include "GraphicsBuffer.h"
#include "VertexLayout.h"


namespace chisel
{
    struct VertexBuffer final : GraphicsBuffer
    {
        VertexLayout layout;
        union {
            const void* pointer;
            const float* vertices;
        };

        VertexBuffer() {}

        VertexBuffer(VertexLayout layout, const void* vertices, size_t size)
          : GraphicsBuffer(size / layout.Stride()),
            layout(layout),
            pointer(vertices)
        {}

        inline size_t Stride() const override final {
            return layout.Stride();
        }
    };


}