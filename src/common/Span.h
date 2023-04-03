#pragma once

#include "Common.h"
#include <span>

namespace chisel
{
    // Use std::span for now...
    template <typename T>
    using Span = std::span<T>;

    using Buffer = Span<byte>;
}
