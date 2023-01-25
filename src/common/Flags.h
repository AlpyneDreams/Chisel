#pragma once

#include "Enum.h"

namespace engine
{
    /** Wraps a bitfield (flags) enum.
     * Set Flag: |= flag
     * Clear Flag: &= ~flag
     * Has Any: (value | mask) != 0 
     * Has All: (value | mask) == mask
    */
    template <EnumType E>
    class Flags
    {
        using Int = std::underlying_type_t<E>;
        
        Int value;

    public:
        // Constructors
        Flags() {}
        Flags(Int value) : value(value) {}
        Flags(E value) : value(value) {}
        Flags(E flag, auto... flags) : value((flag | ... | flags)) {}

        // Modifiers
        Flags& Set(auto... flags)   { value |= (flags | ...); return *this; }
        Flags& Clear(auto... flags) { value &= ~(flags | ...); return *this; }
        Flags& Set(const Flags& other)   { value |= other.value; return *this; }
        Flags& Clear(const Flags& other) { value &= ~other.value; return *this; }
        Flags& Clear() { value = 0; return *this; }

        // Accessors
        bool Empty() const { return value == 0; }
        bool Has(auto... flag) const { return Any(flag...); }
        bool Any(auto... flags) const { return (value & (flags | ...)) != 0; }
        bool All(auto... flags) const { return (value & (flags | ...)) == (flags | ...); }

        // Operators
        Flags operator & (const Flags& other) const {return Flags(value & other.value);}
        Flags operator | (const Flags& other) const {return Flags(value | other.value);}
        Flags operator ^ (const Flags& other) const {return Flags(value ^ other.value);}
        bool operator == (const Flags& other) const {return value == other.value;}
        bool operator != (const Flags& other) const {return value != other.value;}

        // Conversion
        operator Int() const { return value; }
        operator E() const { return E(value); }

        // Indexing
        constexpr bool operator [](E e) const { return (value & e) != 0; }
        constexpr bool operator [](Int e) const { return (value & e) != 0; }
        constexpr bool operator [](auto e) const { return (value & e) != 0; }
        auto operator [](E e) { return (value & e) != 0; }
        auto operator [](Int e) { return (value & e) != 0; }
        auto operator [](auto e) { return (value & e) != 0; }
    };
}