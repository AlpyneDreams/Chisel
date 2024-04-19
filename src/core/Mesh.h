#pragma once

#include "common/Common.h"
#include "assets/Asset.h"
#include "math/Math.h"
#include "VertexLayout.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <vector>

namespace chisel
{
    struct Material;

    // TODO: Don't store all the vertex data on the CPU here.
    // Just dump it when we dont need it.
    struct Mesh : Asset
    {
        struct Group {
            // TODO: Allow sharing buffers between groups but drawing different ranges
            VertexBuffer vertices = VertexBuffer();
            IndexBuffer indices = IndexBuffer();
            int material = -1;
        };

        std::vector<Group> groups;
        std::vector<Rc<Material>> materials;
        bool uploaded = false;

        using Asset::Asset;

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

        Mesh& operator=(const Mesh& other) {
            if (uploaded) {
                // TODO: Delete GPU buffers
            }
            groups = other.groups;
            materials = other.materials;
            uploaded = false;
            return *this;
        }

    protected:
        template <typename Vertex, size_t V, typename Index, size_t I>
        void Init(const VertexLayout& layout, Vertex (&vertices)[V], Index (&indices)[I])
        {
            if (V == 0 || I == 0)
                return;
            groups.push_back(Group {
                VertexBuffer(layout, vertices, sizeof(vertices)),
                IndexBuffer(indices, sizeof(indices))
            });
        }

        // From std::vector
        template <typename Vertex, typename Index>
        void Init(const VertexLayout& layout,
            const std::vector<Vertex>& vertices, size_t vertsOffset,
            const std::vector<Index>& indices, size_t indicesOffset)
        {
            if (indices.empty() || vertices.empty())
                return;
            groups.push_back(Group {
                VertexBuffer(layout, vertices.data() + vertsOffset, (vertices.size() - vertsOffset) * sizeof(Vertex)),
                IndexBuffer(indices.data() + indicesOffset, (indices.size() - indicesOffset) * sizeof(Index))
            });
        }

        template <typename Vertex, size_t V>
        void Init(const VertexLayout& layout, Vertex (&vertices)[V])
        {
            if (V == 0)
                return;
            groups.push_back(Group {
                VertexBuffer(layout, vertices, sizeof(vertices))
            });
        }
    };

    template <typename Vertex, typename Index = uint32>
    struct MeshBuffer : Mesh
    {
        // TODO: Deal with memory ownership!!
        std::vector<Vertex>& vertices = *new std::vector<Vertex>;
        std::vector<Index>& indices = *new std::vector<Index>;
        size_t verticesOffset = 0;
        size_t indicesOffset = 0;
        
        MeshBuffer()
        {}
        
        Group& AddGroup(int material = -1)
        {
            Init(Vertex::Layout, vertices, verticesOffset, indices, indicesOffset);
            verticesOffset = vertices.size();
            indicesOffset = indices.size();
            Group& group = groups[groups.size() - 1];
            group.material = material;
            return group;
        }
    };
}
