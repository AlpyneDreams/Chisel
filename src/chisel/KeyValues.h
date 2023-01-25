#pragma once


// TODO: Replace this with a better, simpler, parser
#include "chisel/deps/vdf_parser.h"

#include "math/Color.h"
#include "common/String.h"

#include <map>
#include <memory>
#include <string>
#include <utility>

namespace chisel::hammer
{
    struct KeyValues
    {
    private:
        using KVObject = std::map<std::string, std::shared_ptr<KeyValues>>;

        inline static const std::string ObjectStr = "[object Object]";
    public:
        enum Type { Null, String, Object };
        Type type = Type::Object;

        std::string name;

    private:
        // TODO: union or variant
        std::string value;

        KVObject object;
        std::shared_ptr<KeyValues> next = nullptr;

        static KeyValues null;
    public:

        KeyValues() {}
        KeyValues(const KeyValues& kv) { *this = kv; }
        KeyValues(Type type) : type(type) {}
        explicit KeyValues(std::string name, std::string value) : type(String), name(name), value(value) {}

        operator std::string() const { return type == Object ? ObjectStr : value; }
        operator std::string_view() const { return type == Object ? ObjectStr : value; }
        operator const char*() const { return type == Object ? ObjectStr.c_str() : value.c_str(); }

        template <typename T>
        operator std::vector<T>();

        operator int() const { return type == String ? std::stoi(*this) : 0; }
        operator float() const { return type == String ? std::stof(*this) : 0.f; }
        operator bool() const { return type == Object || (type == String && std::stoi(*this) != 0); }

        template <typename T>
        operator ColorRGBA<T> () const
        {
            if (type != String)
                return ColorRGBA<T>(0, 0, 0, 0);

            ColorRGBA<T> color;
            auto str = str::split(value);
            for (int i = 0; i < str.size() && i < 4; i++)
                color.data[i] = T(std::stod(std::string(str[i])));

            return color;
        }

        bool operator ==(const KeyValues& kv) const
        {
            if (type != kv.type)
                return false;
            switch (type)
            {
                case String:
                    return value == kv.value;
                case Object:
                    return false; // TODO
                default:
                    return true;
            }
        }

        bool operator !=(const KeyValues& kv) const { return !(*this == kv); }

        static KeyValues Parse(std::string& str)
        {
            return tyti::vdf::read<KeyValues>(str.begin(), str.end());
        }

        KeyValues& Get(const char* key) {
            return object.contains(key) ? *object[key] : null;
        }

        KeyValues& operator [](const char* key) {
            return Get(key);
        }

    // Iterate all child keys. //

        struct Iterator
        {
            KVObject::iterator it;
            Iterator(KVObject::iterator it) : it(it) {}
            auto& operator *() { return *it->second; }
            auto& operator ++() { return ++it; }
            bool operator ==(const Iterator& that) const { return it == that.it; }
        };

        auto begin() { return Iterator(object.begin()); }
        auto end() { return Iterator(object.end()); }

    // Iterate multiple siblings with the same key. //

        struct Iterable
        {
            struct Iterator
            {
                KeyValues* kv;
                Iterator(KeyValues* ptr) : kv(ptr && ptr->type == Null ? nullptr : ptr) {}
                auto& operator *() { return *kv; }
                auto& operator ++()
                {
                    if (kv)
                        kv = kv->next.get();
                    return *this;
                }
                bool operator ==(const Iterator& that) const { return kv == that.kv; }
            };

            KeyValues* kv;
            Iterable(KeyValues* ptr) : kv(ptr) {}
            auto begin() { return Iterator(kv); }
            auto end() { return Iterator(nullptr); }
        };

        Iterable each(const char* key) {
            return Iterable(&Get(key));
        }

        Iterable each() {
            return Iterable(this);
        }

        //Iterable Each(const char* key) { return each(key); }
        //Iterable Each() { return each(); }

        //Iterable operator ()(const char* key) { return each(key); }

    // tyti::vdf API //

        void add_attribute(std::string key, std::string value)
        {
            object.emplace(key, std::make_unique<KeyValues>(key, value));
        }

        void add_child(std::unique_ptr<KeyValues> child)
        {
            std::shared_ptr<KeyValues> obj { child.release() };
            if (!object.contains(obj->name))
            {
                object.emplace(obj->name, obj);
                return;
            }
            else
            {
                auto kv = object[obj->name];
                while (kv->next)
                    kv = kv->next;
                kv->next = obj;
            }
        }

        void set_name(std::string nombre) {
            name = nombre;
        }

    };

    template <typename T>
    KeyValues::operator std::vector<T>()
    {
        std::vector<T> vec;
        // It is possible to have a string key with the same name as an object key,
        // so only include object keys for now.
        // TODO: Can string keys have multiple instances, or only objects?
        for (auto& sibling : each())
            if (sibling.type == Object)
                vec.emplace_back(sibling);
        return vec;
    }


    inline KeyValues KeyValues::null = KeyValues::Null;

}

// FMT/ostream support

namespace chisel::hammer
{
    inline std::ostream& operator<<(std::ostream& os, const KeyValues& kv) {
        return os << kv.operator std::string();
    }
}
template <> struct fmt::formatter<chisel::hammer::KeyValues> : ostream_formatter {};


// All the C++ boilerplate necessary to allow structured binding:
//  auto& [key, value] = kv;

namespace std
{
    template <>
    struct tuple_size<chisel::hammer::KeyValues> {
        static constexpr size_t value = 2;
    };

    template<>
    struct tuple_element<0, chisel::hammer::KeyValues> {
        using type = std::string;
    };

    template<>
    struct tuple_element<1, chisel::hammer::KeyValues> {
        using type = chisel::hammer::KeyValues&;
    };
}

namespace chisel::hammer
{
    template<std::size_t Index>
    std::tuple_element_t<Index, chisel::hammer::KeyValues>& get(chisel::hammer::KeyValues& kv)
    {
        if constexpr (Index == 0) return kv.name;
        if constexpr (Index == 1) return kv;
    }
}
