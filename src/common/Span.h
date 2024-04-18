#pragma once

#include "Common.h"

#include <string>
#include <cstring>
#include <iostream>
#include <vector>

namespace chisel
{
    template <typename T>
    struct Span
    {
        Span(std::nullptr_t) : data(nullptr), size(0) {}

        Span(T* data, size_t size) : data(data), size(size) {}

        Span(T* begin, T* end) : data(begin), size(uintptr_t(end) - uintptr_t(begin)) {}

        template <size_t Count>
        Span(T (&array)[Count]) : data(array), size(Count) {}

        template <typename J>
        Span(J& x)
          : data(x.begin())
          , size(x.end() - x.begin()) {}
        
              T& operator [] (size_t idx)       { return data[idx]; }
        const T& operator [] (size_t idx) const { return data[idx]; }

        template <typename J>
        bool operator == (const Span<J>& other) const
        {
            if (other.size != size)
                return false;

            for (size_t i = 0; i < size; i++)
            {
                if (data[i] != other.data[i])
                    return false;
            }

            return true;
        }

        bool IsEmpty() const { return size == 0; }

              T* begin()       { return &data[0]; }
        const T* begin() const { return &data[0]; }

              T* end()       { return &data[size]; }
        const T* end() const { return &data[size]; }

              T& front()       { return data[0]; }
        const T& front() const { return data[0]; }

              T& back()       { return data[size - 1]; }
        const T& back() const { return data[size - 1]; }

        void copy(void* dst)
        {
            std::memcpy(dst, data, sizeof(T) * size);
        }

        T* data;
        size_t size;
    };

    using Buffer = std::vector<byte>;

    struct BufferView : public Span<byte>
    {
        using Span<byte>::Span;

        BufferView(Buffer& buffer)
            : BufferView(buffer.data(), buffer.data() + buffer.size()) {}
    };

    struct StringView : public Span<const char>
    {
        using Span<const char>::Span;

        StringView(const std::string& string)
            : Span<const char>(string.data(), string.length()) {}

        StringView(std::string_view view)
            : Span<const char>(view.data(), view.data() + view.size()) {}

        StringView(const char* str)
            : Span<const char>{str, strlen(str)} {}

        StringView(const Buffer& buffer)
            : StringView((const char *)buffer.data(), (const char *)(buffer.data() + buffer.size())) {}

        operator std::string_view() { return std::string_view{ data, size }; }
    };

    struct TextBuffer : public Span<char>
    {
        using Span<char>::Span;

        ~TextBuffer() { delete[] data; }
    };
}
