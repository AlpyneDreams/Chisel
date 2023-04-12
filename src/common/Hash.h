#pragma once

#include "common/Common.h"

#include <string_view>
#include <cstring>
#include <ostream>
#include <cctype>

namespace chisel
{
    using Hash = uint32;

    // Standard FNV 1a String Hash Function

    template <typename T> struct FNV_1a;

    template <> struct FNV_1a<uint32> {
        static constexpr uint32 offset = 2166136261u;
        static constexpr uint32 prime  = 16777619u;
    };

    template <> struct FNV_1a<uint64> {
        static constexpr uint64 offset = 14695981039346656037ull;
        static constexpr uint64 prime  = 1099511628211ull;
    };

    constexpr Hash HashString(const char* str, size_t count) {
        return !count
            ? FNV_1a<Hash>::offset
            : (HashString(str, count - 1) ^ str[count-1]) * FNV_1a<Hash>::prime;
    }

    constexpr Hash HashString(std::string_view str) {
        return HashString(str.data(), str.size());
    }

    constexpr Hash HashStringLower(const char* str, size_t count) {
        return !count
            ? FNV_1a<Hash>::offset
            : (HashStringLower(str, count - 1) ^ std::tolower(str[count-1])) * FNV_1a<Hash>::prime;
    }

    constexpr Hash HashStringLower(std::string_view str) {
        return HashStringLower(str.data(), str.size());
    }

    struct HashedString
    {
        Hash hash;
        std::string_view str;
        constexpr HashedString() {}
        constexpr HashedString(std::string_view str) : hash(HashString(str)), str(str) {}
        constexpr HashedString(const char* str, size_t size) : HashedString(std::string_view(str, size)) {}
        explicit constexpr HashedString(const char* ch) : HashedString(ch, 1) {}
        //explicit HashedString(const char* str) : HashedString(str, std::strlen(str)) {}

        //constexpr HashedString(HashedString& copy) : hash(copy.hash), str(copy.str) {}

        constexpr operator Hash() const { return hash; }
        operator std::string_view() const { return str; }

        template <size_t N>
        constexpr bool operator==(const char (&other)[N]) const { return str == other; }
    };

    constexpr HashedString operator""_h(const char* str, size_t size) {
        return HashedString(str, size);
    }

    constexpr Hash operator""_hash(const char* str, size_t size) {
        return HashedString(str, size).hash;
    }

}

template<>
struct std::hash<chisel::HashedString>
{
    std::size_t operator()(chisel::HashedString const& s) const noexcept
    {
        return s.hash;
    }
};

inline std::ostream& operator<<(std::ostream& os, const chisel::HashedString& h) {
    return os << h.str;
}
