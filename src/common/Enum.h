#pragma once

#include "Common.h"
#include <type_traits>
#include <bitset>

namespace chisel
{
    template <typename T>
    concept EnumType = std::is_enum_v<T>;

    /** Bitset to use a regular 0..N enum as if it were bit flags.
     *  It is important to specify a Max value.
     */
    template <EnumType E, E Max = E::Max>
    class EnumSet
    {
        using Int = std::underlying_type_t<E>;

        std::bitset<size_t(Max)> bits;

    public:
        // Constructors
        EnumSet() {}
        EnumSet(Int value) : bits(value) {}
        EnumSet(E value) : bits(value) {}
        EnumSet(E flag, auto... flags) : bits(flag) { Set(flags...);  }

        // Modifiers
        EnumSet& Set(auto... flags) { (bits.set(flags), ...); return *this; }
        EnumSet& Clear(auto... flags) { (bits.reset(flags), ...);  return *this; }
        EnumSet& Clear() { bits.reset(); return *this; }
        EnumSet& Reset() { return Clear(); }

        // Accessors
        bool Empty() const { return bits.none(); }
        bool Has(auto... flags) const { return Any(flags...); }
        bool Any(auto... flags) const { return (bits[flags] || ...); }
        bool All(auto... flags) const { return (bits[flags] && ...); }

        // Operators
        EnumSet operator & (const EnumSet& other) const {return EnumSet(bits & other.bits);}
        EnumSet operator | (const EnumSet& other) const {return EnumSet(bits | other.bits);}
        EnumSet operator ^ (const EnumSet& other) const {return EnumSet(bits ^ other.bits);}
        bool operator == (const EnumSet& other) const {return bits == other.bits;}
        bool operator != (const EnumSet& other) const {return bits != other.bits;}

        // Conversion
        operator Int() const { return bits.to_ullong(); }
        operator E() const { return bits.to_ullong(); }

        // Indexing
        constexpr bool operator [](E e) const { return bits[size_t(e)]; }
        constexpr bool operator [](Int e) const { return bits[size_t(e)]; }
        constexpr bool operator [](auto e) const { return bits[size_t(e)]; }
        auto operator [](E e) { return bits[size_t(e)]; }
        auto operator [](Int e) { return bits[size_t(e)]; }
        auto operator [](auto e) { return bits[size_t(e)]; }
    };
}