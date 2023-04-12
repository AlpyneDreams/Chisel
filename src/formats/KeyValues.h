#pragma once

#include "common/Variant.h"
#include "common/SmallVector.h"
#include "common/Parse.h"
#include "common/Span.h"
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
            Color,
            Uint64,
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
        using KeyValuesString = SmallVector<char, 8>;

        KeyValuesVariant()
        {
        }

        template <typename T>
        KeyValuesVariant(const T& arg)
        {
            Set(arg);
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
            switch (m_type)
            {
                case Types::String:
                    m_str.clear();
                    break;
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

        operator StringView();
        operator std::string_view() const;
        operator int64_t();
        operator int32_t();
        operator bool();
        operator float();
        operator double();
        operator void*();
        operator Color255();
        operator uint32_t();
        operator uint64_t();
        operator KeyValues&();

        void Set(const std::string& val) { Clear(); m_type = Types::String;    m_str = KeyValuesString{ StringView{ val } }; }
        void Set(std::string_view val)   { Clear(); m_type = Types::String;    m_str = KeyValuesString{ StringView{ val } }; }
        void Set(StringView val)         { Clear(); m_type = Types::String;    m_str = KeyValuesString{ val }; }
        void Set(int64_t val)            { Clear(); m_type = Types::Int;       m_data.Get<int64_t>() = val; }
        void Set(int32_t val)            { Clear(); m_type = Types::Int;       m_data.Get<int64_t>() = val; }
        void Set(bool val)               { Clear(); m_type = Types::Int;       m_data.Get<int64_t>() = val ? 1 : 0; }
        void Set(float val)              { Clear(); m_type = Types::Float;     m_data.Get<double>() = val; }
        void Set(double val)             { Clear(); m_type = Types::Float;     m_data.Get<double>() = val; }
        void Set(void* val)              { Clear(); m_type = Types::Ptr;       m_data.Get<void*>() = val; }
        void Set(Color255 val)           { Clear(); m_type = Types::Color;     m_data.Construct<Color255>(val); }
        void Set(uint32_t val)           { Clear(); m_type = Types::Uint64;    m_data.Get<uint64_t>() = val; }
        void Set(uint64_t val)           { Clear(); m_type = Types::Uint64;    m_data.Get<uint64_t>() = val; }
        void Set(KeyValuesChild val)     { Clear(); m_type = Types::KeyValues; m_data.Get<KeyValuesChild>() = std::move(val); }

        KeyValuesVariant& operator [](const char *string);

        const KeyValuesVariant& operator [](const char *string) const;

        static KeyValuesVariant& GetEmptyValue() { return s_Nothing; }

        KeyValuesType GetType() const { return m_type; }
    private:
        static KeyValuesVariant s_Nothing;

        KeyValuesType m_type = Types::None;

        KeyValuesString m_str;

        Variant<
            int64_t,
            double,
            void*,
            uint64_t,
            Color255,
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

        KeyValuesVariant& operator [](const char *string)
        {
            auto iter = m_children.find(string);
            if (iter == m_children.end())
            {
                //std::cerr << "Returning KeyValuesVariant empty value: " << string << std::endl;
                return KeyValuesVariant::GetEmptyValue();
            }

            return iter->second;
        }

        const KeyValuesVariant& operator [] (const char *string) const
        {
            auto iter = m_children.find(string);
            if (iter == m_children.end())
            {
                //std::cerr << "Returning KeyValuesVariant empty value: " << string << std::endl;
                return KeyValuesVariant::GetEmptyValue();
            }

            return iter->second;
        }

        auto FindAll(const char *string)
        {
            return m_children.equal_range(string);
        }

        auto begin() { return m_children.begin(); }
        auto end()   { return m_children.end(); }

        KeyValues()
        {
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
                            kv->m_children.emplace(key, value);
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

    template <>
    inline StringView KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
            case Types::String:
                return m_str;
            default:
                return "";
        }
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
            case Types::String:
                return *stream::Parse<uint64_t>(m_str);
            default:
                return 0;
        }
    }

    template <>
    inline int64_t KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
            case Types::String:
                return *stream::Parse<int64_t>(m_str);
            default:
                return 0;
        }
    }

    template <>
    inline int32_t KeyValuesVariant::Get() const
    {
        return Get<int64_t>();
    }

    template <>
    inline uint32_t KeyValuesVariant::Get() const
    {
        return Get<uint64_t>();
    }

    template <>
    inline bool KeyValuesVariant::Get() const
    {
        return Get<int64_t>() != 0;
    }

    template <>
    inline float KeyValuesVariant::Get() const
    {
        switch (m_type)
        {
            case Types::String:
                return *stream::Parse<float>(m_str);
            default:
                return 0;
        }
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
    inline Color255 KeyValuesVariant::Get() const
    {
        return Color255();
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

    inline KeyValuesVariant::operator StringView()
    {
        return Get<StringView>();
    }
    inline KeyValuesVariant::operator std::string_view() const
    {
        return (std::string_view)Get<StringView>();
    }
    inline KeyValuesVariant::operator int64_t()
    {
        return Get<int64_t>();
    }
    inline KeyValuesVariant::operator int32_t()
    {
        return Get<int32_t>();
    }
    inline KeyValuesVariant::operator bool()
    {
        return Get<bool>();
    }
    inline KeyValuesVariant::operator float()
    {
        return Get<float>();
    }
    inline KeyValuesVariant::operator double()
    {
        return Get<double>();
    }
    inline KeyValuesVariant::operator void*()
    {
        return Get<void*>();
    }
    inline KeyValuesVariant::operator Color255()
    {
        return Get<Color255>();
    }
    inline KeyValuesVariant::operator uint32_t()
    {
        return Get<uint32_t>();
    }
    inline KeyValuesVariant::operator uint64_t()
    {
        return Get<uint64_t>();
    }
    inline KeyValuesVariant::operator KeyValues&()
    {
        return Get<kv::KeyValues&>();
    }
}
