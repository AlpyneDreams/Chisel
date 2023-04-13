#pragma once

#include "common/AlignedStorage.h"

#include <type_traits>
#include <cstring>
#include <algorithm>

namespace chisel
{
    template <typename... Types>
    class Variant
    {
    public:
        static constexpr size_t MaxElementSize = std::max({sizeof(Types)...});
        static constexpr size_t Alignment      = std::max({alignof(Types)...});

        explicit Variant()
        {
        }

        Variant(Variant&& other)
        {
            memcpy(&data, &other.data, sizeof(data));
        }

        Variant& operator = (Variant&& other)
        {
            memcpy(&data, &other.data, sizeof(data));
            return *this;
        }

        template <typename T, typename... Args>
        Variant(Args&&... args)
        {
            Construct<T>(std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        void Construct(Args&&... args)
        {
            new (Ptr<T>()) T(std::forward<Args>(args)...);
        }

        template <typename T>
        void Destruct()
        {
            Ptr<T>()->~T();
        }

        void Clear()
        {
            std::memset(&data, 0, sizeof(data));
        }

        template <typename T>       T* Ptr()       { return reinterpret_cast<T*>      (data.data); }
        template <typename T> const T* Ptr() const { return reinterpret_cast<const T*>(data.data); }

        template <typename T>       T& Get()       { return *Ptr<T>(); }
        template <typename T> const T& Get() const { return *Ptr<T>(); }
    private:
        AlignedStorage<MaxElementSize, Alignment> data;
    };
}
