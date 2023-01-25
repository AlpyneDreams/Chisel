#pragma once

#include "common/Common.h"
#include "math/Math.h"
#include "VertexLayout.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace chisel
{
    struct Mesh
    {
        struct Group {
            VertexBuffer vertices;
            IndexBuffer indices;
        };

        std::vector<Group> groups;
        bool uploaded = false;

        Mesh() {}

        Group& AddGroup() {
            Group& group = groups.emplace_back();
            return group;
        }

        Mesh(VertexLayout& layout, const void* vertices, size_t vSize, const auto* indices, size_t iSize)
        {
            groups.push_back(Group {
                VertexBuffer(layout, vertices, vSize),
                IndexBuffer(indices, iSize)
            });
        }

        // Mesh without index buffer
        Mesh(VertexLayout& layout, const void* vertices, size_t vSize)
        {
            groups.push_back(Group {
                VertexBuffer(layout, vertices, vSize),
                IndexBuffer()
            });
        }

    };



}