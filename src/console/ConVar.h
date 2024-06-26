#pragma once

#include "Console.h"
#include "ConCommand.h"
#include "common/String.h"
#include "common/Ranges.h"
#include "common/Enum.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <charconv>
#include <limits>
#include <type_traits>

namespace chisel
{
    /** Represents a console variable. */
    template <typename T = const char*>
    struct ConVar : public ConCommand
    {
    protected:
        using CallbackFunc = std::function<void(T&)>;

        CallbackFunc callback;
        bool inCallback = false;
    
    public:
        T value;
        T defaultValue;

        ConVar(const char* name, T defaultValue, const char* description, CallbackFunc func)
          : ConCommand(name, description, std::function<void(ConCmd&)>{}, NULL),
            value(defaultValue),
            defaultValue(defaultValue),
            callback(func)
        {}
        
        ConVar(const char* name, T defaultValue, const char* description)
          : ConCommand(name, description, std::function<void(ConCmd&)>{}, NULL),
            value(defaultValue),
            defaultValue(defaultValue)
        {}

        // Cannot call a ConVar like a function (can still Invoke)
        void operator()(auto... args) = delete;
        
        inline void SetValue(T t)
        {
            value = t;
            
            // Allow callback to set the value without recursing!
            if (!inCallback && callback)
            {
                inCallback = true;
                callback(value);
                inCallback = false;
            }
        }

        // Print or set value
        void Invoke(ConCmd& cmd) final override
        {
            using namespace std;
            if (cmd.argc == 0) {
                PrintHelp();
            } else try {
                SetValue(ParseValue(cmd.args));
                PrintValue();
            } catch (invalid_argument const& err) {
                Console.Error("Invalid {} value '{}'", DescribeType(), cmd.args);
            } catch (out_of_range const& err) {
                if constexpr (std::is_integral_v<T>) {
                    Console.Error("Invalid value '{}' out of range ({}, {})", cmd.args,
                        to_string(numeric_limits<T>::lowest()),
                        to_string(numeric_limits<T>::max())
                    );
                }
            }
        }

        static T ParseValue(const char* string);

        static constexpr const char* DescribeType()
        {
            if constexpr (std::is_same_v<T, bool>) {
                return "bool";
            } else if constexpr (std::is_integral_v<T>) {
                return "integer";
            } else if constexpr (std::is_floating_point_v<T>) {
                return "number";
            } else {
                return "string";
            }
        }

        ConVar<T>& operator =(const T& t) { SetValue(t); return *this; }
        operator T() const { return value; }
        bool operator ==(const T& t) { return value == t; }
        auto operator <=>(const T& t) { return value <=> t; }

        void PrintHelp() final override
        {
            PrintValue();
            Console.Log("- {}: {}", DescribeType(), description);
        }
        
        void PrintValue()
        {
            Console.Log("{} = {}", name, ToUnderlying(value));
        }
    };

    template <>
    inline const char* ConVar<const char*>::ParseValue(const char *string) { return string; }

// Booleans //

    // Allowed values: 1, 0, true, false, yes, no, t, f, y, n
    template <>
    inline bool ConVar<bool>::ParseValue(const char *string)
    {
        std::string lower = str::toLower(string);
        std::string_view s = str::trim(lower);
        if (std::isalpha(s[0])) {
            constexpr std::string_view yes[] = {"true", "t", "yes", "y"};
            constexpr std::string_view no[] = {"false", "f", "no", "n"};
            if (util::contains(yes, s)) {
                return true;
            } else if (util::contains(no, s)) {
                return false;
            } else {
                throw std::invalid_argument(string);
            }
        } else try {
            double num = ConVar<double>::ParseValue(string);
            if (num == 1) {
                return true;
            } else if (num == 0) {
                return false;
            } else {
                throw std::invalid_argument(string);
            }
        } catch (std::out_of_range const&) {
            throw std::invalid_argument(string);
        }
    }

// Generic //

    template <typename T>
    inline T ParseValue(std::string_view string)
    {
        UnderlyingType<T> value;
        auto [ptr, err] = std::from_chars(string.data(), string.data() + string.size(), value);
        switch (err)
        {
            case std::errc::invalid_argument:
                throw std::invalid_argument(std::string(string));
            case std::errc::result_out_of_range:
                throw std::out_of_range(std::string(string));
            default:
                return T(value);
        }
    }

    template <typename T> inline T ConVar<T>::ParseValue(const char* string) { return chisel::ParseValue<T>(string); }

// Vectors //

    template <int N, typename T>
    inline glm::vec<N, T> ParseVec(const char* string)
    {
        auto str = str::split(string, " ");

        if (str.size() == 1) // splat
            return glm::vec<N, T>(ParseValue<T>(str[0]));

        glm::vec<N, T> vec;
        for (int i = 0; i < str.size() && i < N; i++)
            vec[i] = ParseValue<T>(str[i]);
        return vec;
    }

    template <> inline vec4 ConVar<vec4>::ParseValue(const char* string) { return ParseVec<4, float>(string); }
    template <> inline vec3 ConVar<vec3>::ParseValue(const char* string) { return ParseVec<3, float>(string); }
    template <> inline vec2 ConVar<vec2>::ParseValue(const char* string) { return ParseVec<2, float>(string); }

    template <> constexpr const char* ConVar<vec4>::DescribeType() { return "vec4"; }
    template <> constexpr const char* ConVar<vec3>::DescribeType() { return "vec3"; }
    template <> constexpr const char* ConVar<vec2>::DescribeType() { return "vec2"; }

    template <> inline void ConVar<vec4>::PrintValue() { Console.Log("{} = {} {} {} {}", name, value.x, value.y, value.z, value.w); }
    template <> inline void ConVar<vec3>::PrintValue() { Console.Log("{} = {} {} {}", name, value.x, value.y, value.z); }
    template <> inline void ConVar<vec2>::PrintValue() { Console.Log("{} = {} {}", name, value.x, value.y); }
}
