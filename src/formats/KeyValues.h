#pragma once

#include "common/Variant.h"
#include "common/SmallVector.h"
#include "common/Parse.h"
#include "common/Span.h"
#include "common/String.h"
#include "math/Color.h"

#include <unordered_map>
#include <cstdint>
#include <string>
#include <cstring>
#include <memory>

namespace chisel::kv
{
    namespace Types
    {
        enum KVVariantType
        {
            None,
            String,
            Int,
            Float,
            Ptr,
            Vector2,
            Vector3,
            Vector4,
            KeyValues,

            Count,
        };
    }
    using KeyValuesType = Types::KVVariantType;

    class KeyValues;

    #define fast_toupper( c ) ( ( ( (c) >= 'a' ) && ( (c) <= 'z' ) ) ? ( (c) - 32 ) : (c) )
    #define fast_tolower( c ) ( ( ( (c) >= 'A' ) && ( (c) <= 'Z' ) ) ? ( (c) + 32 ) : (c) )

    template <typename T>
    static void hash_combine(size_t& s, const T& v)
    {
	    std::hash<T> h;
	    s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
    }

    struct KVStringHash
    {
        size_t operator()(const std::string& key) const
        {
            size_t hash = 0;
            for (char c : key)
                hash_combine(hash, fast_tolower(c));
            return hash;
        }
    };

    struct KVStringEqual
    {
        bool operator()(const std::string& a, const std::string& b) const
        {
            if (a.size() != b.size())
                return false;

            return std::equal(a.begin(), a.end(), b.begin(), [](char a, char b) { return fast_tolower(a) == fast_tolower(b); });
        }
    };

    class KeyValuesVariant
    {
    public:
        using KeyValuesChild = std::unique_ptr<KeyValues>;
        using KeyValuesString = std::string;

        KeyValuesVariant()
        {
        }

        template <typename T>
        KeyValuesVariant(const T& arg)
        {
            Set(arg);
        }

        KeyValuesVariant(KeyValuesVariant&& other)
            : m_type(other.m_type)
            , m_str(std::move(other.m_str))
            , m_data(std::move(other.m_data))
        {
            other.m_type = Types::None;
        }

        template <typename T>
        KeyValuesVariant(T&& arg)
        {
            Set(std::move(arg));
        }

        ~KeyValuesVariant()
        {
            Clear();
        }

        void Clear()
        {
            m_str.clear();

            switch (m_type)
            {
                case Types::KeyValues:
                    m_data.Destruct<KeyValuesChild>();
                    break;
                default:
                    break;
            }
            m_type = Types::None;
            m_data.Clear();
        }

        template <typename T>
        T Get() const;

        operator std::string_view() const;
        operator int64_t() const ;
        operator int32_t() const;
        operator bool() const;
        operator float() const;
        operator double() const;
        operator void*() const;
        operator vec2() const;
        operator vec3() const;
        operator vec4() const;
        operator uint32_t() const;
        operator uint64_t() const;
        operator KeyValues&() const;

        void Set(const std::string& val) { Clear(); m_type = Types::String;    m_str = KeyValuesString{ val }; }
        void Set(std::string_view val)   { Clear(); m_type = Types::String;    m_str = KeyValuesString{ val }; }
        void Set(int64_t val)            { Clear(); m_type = Types::Int;       m_data.Get<int64_t>() = val; }
        void Set(int32_t val)            { Clear(); m_type = Types::Int;       m_data.Get<int64_t>() = val; }
        void Set(bool val)               { Clear(); m_type = Types::Int;       m_data.Get<int64_t>() = val ? 1 : 0; }
        void Set(float val)              { Clear(); m_type = Types::Float;     m_data.Get<double>() = val; }
        void Set(double val)             { Clear(); m_type = Types::Float;     m_data.Get<double>() = val; }
        void Set(void* val)              { Clear(); m_type = Types::Ptr;       m_data.Get<void*>() = val; }
        void Set(vec2 val)               { Clear(); m_type = Types::Vector2;   m_data.Get<vec2>() = val; }
        void Set(vec3 val)               { Clear(); m_type = Types::Vector3;   m_data.Get<vec3>() = val; }
        void Set(vec4 val)               { Clear(); m_type = Types::Vector4;   m_data.Get<vec4>() = val; }
        void Set(uint32_t val)           { Clear(); m_type = Types::Int;       m_data.Get<int64_t>() = val; }
        void Set(uint64_t val)           { Clear(); m_type = Types::Int;       m_data.Get<int64_t>() = val; }
        void Set(KeyValuesChild val)     { Clear(); m_type = Types::KeyValues; m_data.Get<KeyValuesChild>() = std::move(val); }

        KeyValuesVariant& operator = (KeyValuesVariant&& other)
        {
            m_type = other.m_type;
            m_str = std::move(other.m_str);
            m_data = std::move(other.m_data);

            other.m_type = Types::None;
            return *this;
        }

        template <typename T>
        KeyValuesVariant& operator = (const T& thing) { Set(thing); return *this; }

        template <typename T>
        bool operator == (const T& thing) const { return ((T)(*this)) == thing; }

        template <typename T>
        bool operator != (const T& thing) const { return ((T)(*this)) != thing; }

        void EnsureType(KeyValuesType type);
        void AssertType(KeyValuesType type) { assert(m_type == type); }

        // Remember to call ValueChanged if you change the value by pointer
        template <typename T>
        T* GetPtr(KeyValuesType type)
        {
            if (m_type != type)
                return nullptr;
            if (m_type == Types::None)
                return nullptr;
            if (m_type == Types::String)
                return reinterpret_cast<T*>(&m_str);
            return &m_data.Get<T>();
        }

        void ValueChanged()
        {
            if (m_type != Types::String)
                m_str.clear();
        }

        KeyValuesVariant& operator [](const char *string);

        const KeyValuesVariant& operator [](const char *string) const;

        static KeyValuesVariant& GetEmptyValue() { return s_Nothing; }

        KeyValuesType GetType() const { return m_type; }

        static KeyValuesVariant Parse(std::string_view view);

        bool IsDefault() const;
    private:
        void UpdateString() const;

        static KeyValuesVariant s_Nothing;

        KeyValuesType m_type = Types::None;

        mutable KeyValuesString m_str;

        Variant<
            int64_t,
            double,
            void*,
            uint64_t,
            vec2,
            vec3,
            vec4,
            KeyValuesChild> m_data;
    };
    inline KeyValuesVariant KeyValuesVariant::s_Nothing;

    class KeyValues
    {
    public:
        static std::unique_ptr<KeyValues> ParseFromUTF8(StringView buffer)
        {
            const char *start = buffer.begin();
            const char *end   = buffer.end();
            return ParseChild(start, end);
        }

        static KeyValues& Nothing()
        {
            return s_Nothing;
        }

        KeyValuesVariant& operator [](std::string_view string)
        {
            auto iter = m_children.find(std::string(string));
            if (iter == m_children.end())
            {
                //std::cerr << "Returning KeyValuesVariant empty value: " << string << std::endl;
                return KeyValuesVariant::GetEmptyValue();
            }

            return iter->second;
        }

        const KeyValuesVariant& operator [] (std::string_view string) const
        {
            auto iter = m_children.find(std::string(string));
            if (iter == m_children.end())
            {
                //std::cerr << "Returning KeyValuesVariant empty value: " << string << std::endl;
                return KeyValuesVariant::GetEmptyValue();
            }

            return iter->second;
        }

        auto FindAll(std::string_view string)
        {
            return m_children.equal_range(std::string(string));
        }

        auto begin() { return m_children.begin(); }
        auto end()   { return m_children.end(); }
        auto begin() const { return m_children.begin(); }
        auto end() const   { return m_children.end(); }

        bool Contains(std::string_view name)
        {
            return m_children.contains(std::string(name));
        }

        KeyValues()
        {
        }

        template <typename... Args>
        KeyValuesVariant& CreateChild(std::string_view name, Args... args)
        {
            return m_children.emplace(std::string(name), KeyValuesVariant::Parse(std::forward<Args>(args)...))->second;
        }

        bool empty() const { return m_children.empty(); }

        void RemoveAll(std::string_view name)
        {
            auto range = m_children.equal_range(std::string(name));
            if (range.first == range.second)
                return;
            m_children.erase(range.first, range.second);
        }
    private:
        static KeyValues s_Nothing;

        static std::unique_ptr<KeyValues> ParseChild(const char*& start, const char* end)
        {
            std::string_view range = std::string_view(start, end);

            if (start == end)
                return nullptr;

            stream::ConsumeSpaceAndNewLine(start, end);

            auto kv = std::make_unique<KeyValues>();

            bool inString = false;

            bool fillingInValue = false;
            std::string key;
            std::string value;

            if (*start == '{')
                start++;

            while (!stream::EndOfStream(start, end))
            {
                if (stream::IsStringToken(range, start))
                {
                    inString = !inString;
                    start++;
                    continue;
                }

                if (!inString)
                {
                    if (stream::IsCPPComment(range, start))
                    {
                        stream::AdvancePast(start, end, stream::NewlineDelimiters);
                        stream::ConsumeSpaceAndNewLine(start, end);
                        continue;
                    }

                    if (*start == '{')
                    {
                        auto child = ParseChild(start, end);
                        kv->m_children.emplace(key, std::move(child));

                        fillingInValue = false;
                        key.clear();
                        value.clear();
                    }

                    // Could be EOS after ParseChild.
                    if (stream::EndOfStream(start, end))
                        break;

                    if (*start == '}')
                    {
                        start++;
                        break;
                    }

                    if (stream::IsWhitespace(*start) || stream::IsNewLine(*start))
                    {
                        fillingInValue = !fillingInValue;

                        if (!fillingInValue)
                        {
                            kv->CreateChild(key, value);
                            key.clear();
                            value.clear();
                        }
                        else if (stream::IsNewLine(*start))
                        {
                            fillingInValue = false;
                        }

                        stream::ConsumeSpaceAndNewLine(start, end);
                        continue;
                    }
                }

                if (fillingInValue)
                    value.push_back(*start);
                else
                    key.push_back(*start);

                // Sanity before advancing.
                if (!stream::EndOfStream(start, end))
                    start++;
            }

            return kv;
        }

        std::unordered_multimap<std::string, KeyValuesVariant, KVStringHash, KVStringEqual> m_children;
    };
    inline KeyValues KeyValues::s_Nothing;

    inline void KeyValuesVariant::UpdateString() const
    {
        auto printKV = [&](char* dst, size_t dst_length)
        {
            int len = 0;
            switch (m_type)
            {
                case Types::Int:       len = snprintf(dst, dst_length, "%lld",            (long long int)m_data.Get<int64_t>()); break;
                case Types::Float:     len = snprintf(dst, dst_length, "%g",              m_data.Get<double>()); break;
                case Types::Ptr:       len = snprintf(dst, dst_length, "%p",              m_data.Get<void*>()); break;
                case Types::Vector2:   len = snprintf(dst, dst_length, "%g %g",           m_data.Get<vec2>()[0], m_data.Get<vec2>()[1]); break;
                case Types::Vector3:   len = snprintf(dst, dst_length, "%g %g %g",        m_data.Get<vec3>()[0], m_data.Get<vec3>()[1], m_data.Get<vec3>()[2]); break;
                case Types::Vector4:   len = snprintf(dst, dst_length, "%g %g %g %g",     m_data.Get<vec4>()[0], m_data.Get<vec4>()[1], m_data.Get<vec4>()[2], m_data.Get<vec4>()[3]); break;
                case Types::KeyValues: len = snprintf(dst, dst_length, "[object Object]"); break;
            }
            return len;
        };

        int len = printKV(nullptr, 0) + 1;
        m_str.resize(len);
        printKV(m_str.data(), m_str.size());
        m_str.resize(len - 1);
    }

    template <>
    inline std::string_view KeyValuesVariant::Get() const
    {
        if (m_type == Types::String)
            return m_str;

        if (m_type == Types::None)
            return "";

        if (!m_str.empty())
            return m_str;

        UpdateString();
        return m_str;
    }

    // todo josh:
    // make the ::Get variant return a result
    // return un-resulted version of that with the
    // operators

    template <>
    inline uint64_t KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
            case Types::String:  return 0;
            case Types::Int:     return (uint64_t)m_data.Get<int64_t>();
            case Types::Float:   return (uint64_t)m_data.Get<double>();
            case Types::Vector2: return (uint64_t)m_data.Get<vec2>()[0];
            case Types::Vector3: return (uint64_t)m_data.Get<vec3>()[0];
            case Types::Vector4: return (uint64_t)m_data.Get<vec4>()[0];
            default:             return 0;
        }
    }

    template <>
    inline int64_t KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
            case Types::String:  return 0;
            case Types::Int:     return (int64_t)m_data.Get<int64_t>();
            case Types::Float:   return (int64_t)m_data.Get<double>();
            case Types::Vector2: return (int64_t)m_data.Get<vec2>()[0];
            case Types::Vector3: return (int64_t)m_data.Get<vec3>()[0];
            case Types::Vector4: return (int64_t)m_data.Get<vec4>()[0];
            default:             return 0;
        }
    }

    template <>
    inline int32_t KeyValuesVariant::Get() const
    {
        return (int32_t)Get<int64_t>();
    }

    template <>
    inline uint32_t KeyValuesVariant::Get() const
    {
        return (uint32_t)Get<uint64_t>();
    }

    template <>
    inline bool KeyValuesVariant::Get() const
    {
        return Get<int64_t>() != 0;
    }

    template <>
    inline double KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
        case Types::String:  return 0.0f;
        case Types::Int:     return (double)m_data.Get<int64_t>();
        case Types::Float:   return (double)m_data.Get<double>();
        case Types::Vector2: return (double)m_data.Get<vec2>()[0];
        case Types::Vector3: return (double)m_data.Get<vec3>()[0];
        case Types::Vector4: return (double)m_data.Get<vec4>()[0];
        default:             return 0;
        }
    }

    template <>
    inline float KeyValuesVariant::Get() const
    {
        return (float)Get<double>();
    }

    template <>
    inline KeyValues& KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
            case Types::KeyValues:
            {
                auto value = m_data.Get<KeyValuesChild>().get();
                if (value)
                    return *value;
            }
            [[fallthrough]];
            default:
                return KeyValues::Nothing();
        }
    }

    template <>
    inline vec2 KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
        case Types::Int:     return vec2(m_data.Get<int64_t>(),  0.0f);
        case Types::Float:   return vec2(m_data.Get<double>(),   0.0f);
        case Types::Vector2: return m_data.Get<vec2>().xy;
        case Types::Vector3: return m_data.Get<vec3>().xy;
        case Types::Vector4: return m_data.Get<vec4>().xy;
        default:             return vec2(0.0f);
        }
    }

    template <>
    inline vec3 KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
        case Types::Int:     return vec3(m_data.Get<int64_t>(),  0.0f, 0.0f);
        case Types::Float:   return vec3(m_data.Get<double>(),   0.0f, 0.0f);
        case Types::Vector2: return vec3(m_data.Get<vec2>().xy, 0.0f);
        case Types::Vector3: return m_data.Get<vec3>().xyz;
        case Types::Vector4: return m_data.Get<vec4>().xyz;
        default:             return vec3(0.0f);
        }
    }

    template <>
    inline vec4 KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
        case Types::Int:     return vec4(m_data.Get<int64_t>(),  0.0f, 0.0f, 0.0f);
        case Types::Float:   return vec4(m_data.Get<double>(),   0.0f, 0.0f, 0.0f);
        case Types::Vector2: return vec4(m_data.Get<vec2>().xy, 0.0f, 0.0f);
        case Types::Vector3: return vec4(m_data.Get<vec3>().xyz, 0.0f);
        case Types::Vector4: return m_data.Get<vec4>().xyzw;
        default:             return vec4(0.0f);
        }
    }

    inline KeyValuesVariant& KeyValuesVariant::operator [](const char *string)
    {
        if (m_type != Types::KeyValues)
            return s_Nothing;

        return Get<kv::KeyValues&>()[string];
    }

    inline const KeyValuesVariant& KeyValuesVariant::operator [](const char *string) const
    {
        if (m_type != Types::KeyValues)
            return s_Nothing;

        return Get<const kv::KeyValues&>()[string];
    }

    inline KeyValuesVariant::operator std::string_view() const
    {
        return Get<std::string_view>();
    }
    inline KeyValuesVariant::operator int64_t() const
    {
        return Get<int64_t>();
    }
    inline KeyValuesVariant::operator int32_t() const
    {
        return Get<int32_t>();
    }
    inline KeyValuesVariant::operator bool() const
    {
        return Get<bool>();
    }
    inline KeyValuesVariant::operator float() const
    {
        return Get<float>();
    }
    inline KeyValuesVariant::operator double() const
    {
        return Get<double>();
    }
    inline KeyValuesVariant::operator void*() const
    {
        return Get<void*>();
    }
    inline KeyValuesVariant::operator vec2() const
    {
        return Get<vec2>();
    }
    inline KeyValuesVariant::operator vec3() const
    {
        return Get<vec3>();
    }
    inline KeyValuesVariant::operator vec4() const
    {
        return Get<vec4>();
    }
    inline KeyValuesVariant::operator uint32_t() const
    {
        return Get<uint32_t>();
    }
    inline KeyValuesVariant::operator uint64_t() const
    {
        return Get<uint64_t>();
    }
    inline KeyValuesVariant::operator KeyValues&() const
    {
        return Get<kv::KeyValues&>();
    }

    inline KeyValuesVariant KeyValuesVariant::Parse(std::string_view view)
    {
        if (view.empty())
            return KeyValuesVariant();

        if (stream::IsPotentiallyNumber(view[0]))
        {
            auto vec = str::split(view, " ");
            if (vec.size() == 1)
            {
                bool hasDecimal = view.find('.') != std::string_view::npos;
                if (hasDecimal)
                {
                    auto r_double = stream::Parse<double>(view);
                    if (r_double) return KeyValuesVariant(*r_double);
                }
                else
                {
                    auto r_int64 = stream::Parse<int64>(view);
                    if (r_int64) return KeyValuesVariant(*r_int64);
                }
            }
            else if (vec.size() == 2)
            {
                auto r_x = stream::Parse<float>(vec[0]);
                auto r_y = stream::Parse<float>(vec[1]);
                if (r_x && r_y)
                    return KeyValuesVariant(vec2(*r_x, *r_y));
            }
            else if (vec.size() == 3)
            {
                auto r_x = stream::Parse<float>(vec[0]);
                auto r_y = stream::Parse<float>(vec[1]);
                auto r_z = stream::Parse<float>(vec[2]);
                if (r_x && r_y && r_z)
                    return KeyValuesVariant(vec3(*r_x, *r_y, *r_z));
            }
            else if (vec.size() == 4)
            {
                auto r_x = stream::Parse<float>(vec[0]);
                auto r_y = stream::Parse<float>(vec[1]);
                auto r_z = stream::Parse<float>(vec[2]);
                auto r_w = stream::Parse<float>(vec[3]);
                if (r_x && r_y && r_z && r_w)
                    return KeyValuesVariant(vec4(*r_x, *r_y, *r_z, *r_w));
            }
        }

        return KeyValuesVariant(view);
    }

    inline void KeyValuesVariant::EnsureType(KeyValuesType type)
    {
        if (m_type == type)
            return;

        if (type == Types::String)
        {
            if (m_str.empty())
                UpdateString();

            m_type = Types::String;
            return;
        }

        if (m_type == Types::None)
        {
            std::memset(&m_data, 0, sizeof(m_data));
            m_type = type;
            return;
        }

        if      (m_type == Types::Float   && type == Types::Vector2) Set(vec2(Get<double>(), 0.0f));
        else if (m_type == Types::Float   && type == Types::Vector3) Set(vec3(Get<double>(), 0.0f, 0.0f));
        else if (m_type == Types::Float   && type == Types::Vector4) Set(vec4(Get<double>(), 0.0f, 0.0f, 0.0f));
        else if (m_type == Types::Vector2 && type == Types::Vector3) Set(vec3(Get<vec2>(), 0.0f));
        else if (m_type == Types::Vector2 && type == Types::Vector4) Set(vec4(Get<vec2>(), 0.0f, 0.0f));
        else if (m_type == Types::Vector3 && type == Types::Vector4) Set(vec4(Get<vec3>(), 0.0f));
        else if (m_type == Types::Int     && type == Types::Float)   Set(double(Get<int>()));
        else
            abort();
    }

    inline bool KeyValuesVariant::IsDefault() const
    {
        switch (m_type)
        {
        default:
        case Types::None:      return true;
        case Types::String:    return m_str.empty();
        case Types::Int:       return m_data.Get<int>() == 0;
        case Types::Float:     return m_data.Get<double>() == 0;
        case Types::Ptr:       return m_data.Get<void*>() == nullptr;
        case Types::Vector2:   return m_data.Get<vec2>() == vec2(0.0f);
        case Types::Vector3:   return m_data.Get<vec3>() == vec3(0.0f);
        case Types::Vector4:   return m_data.Get<vec4>() == vec4(0.0f);
        case Types::KeyValues: return m_data.Get<KeyValuesChild>()->empty();
        }
    }
}
