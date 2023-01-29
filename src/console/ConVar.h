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
        T value;
        T defaultValue;

        ConVar(const char* name, T defaultValue, const char* description, auto... flags)
          : ConCommand(name, description, std::function<void(ConCmd&)>{}, flags...),
            value(defaultValue),
            defaultValue(defaultValue)
        {}

        ConVar(const char* name, T defaultValue, const char* description)
          : ConVar(name, defaultValue, description, NULL)
        {}

        // Cannot call a ConVar like a function (can still Invoke)
        void operator()(auto... args) = delete;

        // Print or set value
        void Invoke(ConCmd& cmd) final override
        {
            using namespace std;
            if (cmd.argc == 0) {
                PrintHelp();
            } else try {
                value = ParseValue(cmd.args);
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
                return "number";
            } else {
                return "string";
            }
        }

        ConVar<T>& operator =(const T& t) { value = t; return *this; }
        operator T() const { return value; }
        bool operator ==(const T& t) { return value == t; }
        auto operator <=>(const T& t) { return value <=> t; }

        void PrintHelp() final override
        {
            PrintValue();
            Console.Log("- {}, {}", description, DescribeType());
        }
        
        void PrintValue()
        {
            Console.Log("{} = {}", name, ToUnderlying(value));
        }
    };

    template <>
    inline const char* ConVar<const char*>::ParseValue(const char *string) { return string; }

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

    template <typename T>
    inline T ConVar<T>::ParseValue(const char* string)
    {
        UnderlyingType<T> value;
        auto [ptr, err] = std::from_chars(string, string + std::strlen(string), value);
        switch (err)
        {
            case std::errc::invalid_argument:
                throw std::invalid_argument(string);
            case std::errc::result_out_of_range:
                throw std::out_of_range(string);
            default:
                return T(value);
        }
    }
}
