#pragma once

#include <functional>

namespace chisel
{
    template <typename T>
    struct Property
    {
        using Getter = std::function<T()>;
        using Setter = std::function<void(const T&)>;
        const Getter get;
        const Setter set;
        
        Property(auto getter) : get(getter), set([](const T&) {}) {}
        Property(Getter g, Setter s) : get(g), set(s) {}
        Property(Setter s, Getter g) : get(g), set(s) {}

        inline operator T() const { return get(); }
        inline T& operator ->() const { return get(); }
        inline T& operator *() const { return get(); }
        inline Property& operator=(const T& value) { set(value); return *this; }
    };
}
