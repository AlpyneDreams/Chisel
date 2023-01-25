#pragma once

/** VTable reflection utils. Should work on MSVC, GCC, and Clang.
 *
 * See: https://medium.com/@calebleak/fast-virtual-functions-hacking-the-vtable-for-fun-and-profit-25c36409c5e0
 */

namespace engine
{
    template <class T>
    struct VTable;

    template <class T>
    constexpr VTable<T>& GetVTable(T* obj)
    {
        VTable<T>** ptr = (VTable<T>**)obj;
        return **ptr;
    }

}