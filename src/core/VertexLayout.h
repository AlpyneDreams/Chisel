#pragma once

#include "common/Common.h"

#include <cstddef>
#include <initializer_list>
#include <vector>
#include <utility>
#include <typeinfo>
#include <typeindex>

namespace chisel
{
    struct VertexAttribute {
        enum Mode { Default, None, Position, Normal, Tangent, Bitangent, Color, Indices, Weight, TexCoord };

        template <typename T>
        static VertexAttribute For(uint dimension, Mode mode = Default, bool normalized = false) {
            return VertexAttribute {sizeof(T), dimension, std::type_index(typeid(T)), normalized, mode};
        }

        uint size;
        uint dimension;
        std::type_index type;
        bool normalized = false;
        Mode mode = Default;
    };

    // TODO: Consider having a template class for this?
    // TODO: Auto-generate this from structs using rain

    class VertexLayout
    {
    public:
        using Attribute = VertexAttribute;

        VertexLayout() {}
        VertexLayout(const VertexLayout& v) : stride(v.stride), layout(v.layout) {}
        VertexLayout(std::initializer_list<Attribute> attrs) {
            for (auto attr : attrs) {
                Add(attr);
            }
        }

        template<typename T>
        VertexLayout& Add(uint dimension, Attribute::Mode mode = Attribute::Default, bool normalized = false) {
            return Add(Attribute{sizeof(T), dimension, std::type_index(typeid(T)), normalized, mode});
        }

        VertexLayout& Add(Attribute attr) {
            layout.push_back(attr);
            stride += attr.size * attr.dimension;
            return *this;
        }

        VertexLayout& Skip(uint bytes) {
            layout.push_back(Attribute{1, bytes, std::type_index(typeid(std::nullptr_t)), false, Attribute::None});
            stride += bytes;
            return *this;
        }

        inline uint Stride() const { return layout.empty() ? 1 : stride; }

        inline const std::vector<Attribute>& Attributes() const { return layout; }
    private:
        uint stride = 0;
        std::vector<Attribute> layout;
    };

}