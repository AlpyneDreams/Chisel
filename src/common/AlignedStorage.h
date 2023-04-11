#pragma once

#include <cstdint>
#include <cstddef>

namespace chisel
{
    template <size_t Length, size_t Alignment>
    struct AlignedStorage
    {
        alignas(Alignment) uint8_t data[Length];
    };
}
