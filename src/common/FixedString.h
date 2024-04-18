#pragma once

#include <string_view>

namespace chisel
{
    // Compile-time string
    template <size_t N>
    struct FixedString
    {
        char value[N];

        constexpr inline FixedString(const char (&str)[N]) noexcept {
            for (size_t i = 0; i < N; i++)
                value[i] = str[i];
        }

        constexpr operator std::string_view() const {
            return std::string_view(value, N-1); // exclude null terminator
        }
    };

}
