#pragma once

#include "core/Mesh.h"
#include "math/Math.h"
#include "assets/Assets.h"

namespace chisel
{
    inline struct Primitives
    {
        struct Vertex;
        static inline Mesh& Cube   = *Assets.Load<Mesh>("models/cube.obj");
        static inline Mesh& Teapot = *Assets.Load<Mesh>("models/teapot.obj");
        static inline Mesh& Plane  = *Assets.Load<Mesh>("models/plane.obj");
        static inline Mesh Line;

        Com<ID3D11Buffer> Quad;

        Primitives()
        {
            auto& g = Line.AddGroup();
            g.vertices.layout.Add<float>(3, VertexAttribute::Position);
            g.vertices.pointer = &lineStart;
            g.vertices.count = 2;
        }

        void Init();

    private:
        vec3 lineStart = Vectors.Zero;
        vec3 lineEnd   = Vectors.Forward;
    } Primitives;

    struct Primitives::Vertex
    {
        vec3 pos;
        vec2 uv;

        static constexpr D3D11_INPUT_ELEMENT_DESC Layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
    };
    static_assert(sizeof(Primitives::Vertex) == sizeof(float) * 5);


    inline void Primitives::Init()
    {
        const Primitives::Vertex QuadVerts[] = {
            { { -0.5, -0.5, 0.0 }, { 0, 1 } },
            { { +0.5, +0.5, 0.0 }, { 1, 0 } },
            { { -0.5, +0.5, 0.0 }, { 0, 0 } },
            { { +0.5, +0.5, 0.0 }, { 1, 0 } },
            { { -0.5, -0.5, 0.0 }, { 0, 1 } },
            { { +0.5, -0.5, 0.0 }, { 1, 1 } },
        };

        D3D11_BUFFER_DESC desc
        {
            .ByteWidth      = 6 * sizeof(Primitives::Vertex),
            .Usage          = D3D11_USAGE_IMMUTABLE,
            .BindFlags      = D3D11_BIND_VERTEX_BUFFER,
            .CPUAccessFlags = 0,
        };
        D3D11_SUBRESOURCE_DATA data
        {
            .pSysMem = QuadVerts,
            .SysMemPitch = 0,
            .SysMemSlicePitch = 0,
        };
        if (FAILED(Tools.rctx.device->CreateBuffer(&desc, &data, &Quad))) {
            Console.Error("Failed to upload quad mesh.");
        }
    }
}
