#pragma once

#include "common/Common.h"
#include "math/Math.h"
#include "VertexLayout.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <vector>

namespace chisel
{
    struct Mesh
    {
        struct Group {
            VertexBuffer vertices;
            IndexBuffer indices = IndexBuffer();
        };

        std::vector<Group> groups;
        bool uploaded = false;

        Mesh() {}

        Group& AddGroup() {
            return groups.emplace_back();
        }

        Mesh(VertexLayout& layout, auto& vertices, auto& indices) {
            Init(layout, vertices, indices);
        }

        // Mesh without index buffer
        Mesh(VertexLayout& layout, auto& vertices) {
            Init(layout, vertices);
        }

        template <typename Vertex, size_t V, typename Index, size_t I>
        void Init(VertexLayout& layout, Vertex (&vertices)[V], Index (&indices)[I])
        {
            groups.push_back(Group {
                VertexBuffer(layout, vertices, sizeof(vertices)),
                IndexBuffer(indices, sizeof(indices))
            });
        }

        // From std::vector
        template <typename Vertex, typename Index>
        void Init(VertexLayout& layout, std::vector<Vertex>& vertices, std::vector<Index>& indices)
        {
            groups.push_back(Group {
                VertexBuffer(layout, vertices.data(), vertices.size() * sizeof(Vertex)),
                IndexBuffer(indices.data(), indices.size() * sizeof(Index))
            });
        }

        template <typename Vertex, size_t V>
        void Init(VertexLayout& layout, Vertex (&vertices)[V])
        {
            groups.push_back(Group {
                VertexBuffer(layout, vertices, sizeof(vertices))
            });
        }

    };
}
