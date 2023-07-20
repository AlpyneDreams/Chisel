#pragma once

#include "core/VertexLayout.h"
#include "math/Math.h"
#include "chisel/Selection.h"
#include "render/Render.h"

#include "../submodules/OffsetAllocator/offsetAllocator.hpp"

namespace chisel
{
    struct BrushEntity;

    struct Atom : Selectable
    {
        Atom(BrushEntity* parent)
            : m_parent(parent)
        {
        }
    protected:
        BrushEntity* m_parent;
    };

    struct VertexSolid
    {        
        vec3 position;
        vec3 normal;
        vec3 uv;
        
        static constexpr D3D11_INPUT_ELEMENT_DESC Layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
    };
    
    struct BrushGPUAllocator
    {
    public:
        static constexpr uint32_t BufferSize = 256 * 1024 * 1024; // 256 mb
        static constexpr uint32_t MaxAllocations = 65535 * 4;

        using Allocation = OffsetAllocator::Allocation;
        
        BrushGPUAllocator(render::RenderContext& rctx)
            : m_rctx     (rctx)
            , m_allocator(BufferSize, MaxAllocations)
        {
            D3D11_BUFFER_DESC desc
            {
                .ByteWidth      = BufferSize,
                .Usage          = D3D11_USAGE_DYNAMIC,
                .BindFlags      = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
            };
            m_rctx.device->CreateBuffer(&desc, nullptr, &m_buffer);
        }

        void open()
        {
            if (m_refs++ == 0)
            {
                assert(m_base == nullptr);

                D3D11_MAPPED_SUBRESOURCE mapped;
                HRESULT hr = m_rctx.ctx->Map(m_buffer.ptr(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped);
                if (FAILED(hr))
                {
                    abort();
                }
                m_base = (uint8_t*)mapped.pData;
            }
        }

        void close()
        {
            if (--m_refs == 0)
            {
                assert(m_base != nullptr);
                m_rctx.ctx->Unmap(m_buffer.ptr(), 0);
                m_base = nullptr;
            }
        }

        uint8_t* data()
        {
            assert(m_base != nullptr);
            return m_base;
        }

        Allocation alloc(uint32_t size)
        {
            return m_allocator.allocate(size);
        }

        void free(Allocation alloc)
        {
            m_allocator.free(alloc);
        }

        render::RenderContext& rctx() const { return m_rctx; }

        ID3D11Buffer* buffer() const { return m_buffer.ptr(); }

    private:
        render::RenderContext&     m_rctx;
        OffsetAllocator::Allocator m_allocator;

        Com<ID3D11Buffer>          m_buffer;
        uint8_t*                   m_base = nullptr;
        uint32_t                   m_refs = 0;
    };
}
