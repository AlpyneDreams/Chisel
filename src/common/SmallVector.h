#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

#include "common/AlignedStorage.h"
#include "common/Span.h"

namespace chisel
{
    template <typename T, size_t N>
    class SmallVector
    {
    public:

        SmallVector() {}
        SmallVector(size_t size) { resize(size); }
        SmallVector(Span<const T> span)
        {
            reserve(span.size);
            for (const auto& val : span)
                push_back(val);
        }
        constexpr SmallVector(std::initializer_list<T> list)
        {
            reserve(list.size());
            for (const auto& val : list)
                push_back(val);
        }

        SmallVector             (const SmallVector& x)
        {
            reserve(x.size());
            for (const auto& val : x)
                push_back(val);
        }
        SmallVector& operator = (const SmallVector& x)
        {
            clear();
            reserve(x.size());
            for (const auto& val : x)
                push_back(val);
            return *this;
        }

        ~SmallVector()
        {
            for (size_t i = 0; i < m_size; i++)
                Ptr(i)->~T();

            if (m_capacity > N)
                delete[] u.m_ptr;
        }

        template <typename J>
        bool operator == (const Span<J>& x)
        {
            return Span<T>(*this) == x;
        }

        bool operator == (const char *x)
        {
            return Span<T>(*this) == StringView(x);
        }

        void reserve(size_t n)
        {
            n = PickCapacity(n);

            if (n <= m_capacity)
                return;

            Storage* data = new Storage[n];

            for (size_t i = 0; i < m_size; i++)
            {
                new (&data[i]) T(std::move(*Ptr(i)));
                Ptr(i)->~T();
            }

            if (m_capacity > N)
                delete[] u.m_ptr;

            m_capacity = n;
            u.m_ptr = data;
        }

        size_t size() const   { return m_size; }

        const T* data() const { return Ptr(0); }
              T* data()       { return Ptr(0); }

        void clear()
        {
            resize(0);
        }

        void resize(size_t n)
        {
            reserve(n);

            for (size_t i = n; i < m_size; i++)
                Ptr(i)->~T();

            for (size_t i = m_size; i < n; i++)
                new (Ptr(i)) T();

            m_size = n;
        }

        size_t push_back(const T& object)
        {
            reserve(m_size + 1);
            new (Ptr(m_size++)) T(object);
            return m_size - 1;
        }

        size_t push_back(T&& object)
        {
            reserve(m_size + 1);
            new (Ptr(m_size++)) T(Move(object));
            return m_size - 1;
        }

        template<typename... Args>
        size_t emplace_back(Args... args)
        {
            reserve(m_size + 1);
            new (Ptr(m_size++)) T(Forward<Args>(args)...);
            return m_size - 1;
        }

        void erase(size_t idx)
        {
            Ptr(idx)->~T();

            for (size_t i = idx; i < m_size - 1; i++)
            {
                new (Ptr(i)) T(Move(*Ptr(i + 1)));
                Ptr(i + 1)->~T();
            }
        }

        void pop_back()
        {
            Ptr(--m_size)->~T();
        }

        bool empty() const
        {
            return size() == 0;
        }

        constexpr static size_t InvalidIdx = ~size_t(0);

        size_t find_idx(const T& x) const
        {
            for (size_t i = 0; i < m_size; i++)
            {
                if (*Ptr(i) == x)
                    return i;
            }

            return InvalidIdx;
        }

        bool contains(const T& x) const
        {
            return find_idx(x) != InvalidIdx;
        }

        void copy(void* dst)
        {
            memcpy(dst, Ptr(0), sizeof(T) * m_size);
        }

              T& operator [] (size_t idx)       { return *Ptr(idx); }
        const T& operator [] (size_t idx) const { return *Ptr(idx); }

              T* begin()       { return Ptr(0); }
        const T* begin() const { return Ptr(0); }

              T* end()       { return Ptr(m_size); }
        const T* end() const { return Ptr(m_size); }

              T& front()       { return *Ptr(0); }
        const T& front() const { return *Ptr(0); }

              T& back()       { return *Ptr(m_size - 1); }
        const T& back() const { return *Ptr(m_size - 1); }

        operator Span<T>() { return Span<T>(*this); }

        operator BufferView() { return BufferView{ reinterpret_cast<uint8_t *>(Ptr(0)), sizeof(T) * m_size}; }

    private:
        using Storage = AlignedStorage<sizeof(T), alignof(T)>;

        size_t m_capacity = N;
        size_t m_size     = 0;

        union
        {
            Storage* m_ptr;
            Storage  m_data[sizeof(T) * N];
        } u;

        size_t PickCapacity(size_t n) {
            size_t capacity = m_capacity;

            while (capacity < n)
                capacity = (capacity * 2) + 2;

            return capacity;
        }

        T* Ptr(size_t idx) {
            return m_capacity == N
            ? reinterpret_cast<T*>(&u.m_data[idx])
            : reinterpret_cast<T*>(&u.m_ptr[idx]);
        }

        const T* Ptr(size_t idx) const
        {
            return m_capacity == N
            ? reinterpret_cast<const T*>(&u.m_data[idx])
            : reinterpret_cast<const T*>(&u.m_ptr[idx]);
        }

    };

}
