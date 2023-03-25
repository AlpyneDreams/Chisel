#pragma once

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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
    };

}

namespace chisel::str
{
    // Returns lowercased copy of str
    inline std::string toLower(std::string_view str)
    {
        std::string s = std::string(str);
        std::transform(s.begin(), s.end(), s.begin(), [](auto c) { return std::tolower(c); });
        return s;
    }

    // Returns uppercased copy of str
    inline std::string toUpper(std::string_view str)
    {
        std::string s = std::string(str);
        std::transform(s.begin(), s.end(), s.begin(), [](auto c) { return std::toupper(c); });
        return s;
    }

    // Trim characters from left side of string
    constexpr std::string_view trimStart(std::string_view str, std::string_view chars = " \n\r")
    {
        str.remove_prefix(std::min(str.find_first_not_of(chars), str.size()));
        return str;
    }

    // Trim characters from right side of string
    constexpr std::string_view trimEnd(std::string_view str, std::string_view chars = " \n\r")
    {
        auto pos = str.find_last_not_of(chars);
        if (pos != str.npos)
            str.remove_suffix(str.size() - pos - 1);
        return str;
    }

    // Trim characters surrounding string
    constexpr std::string_view trim(std::string_view str, std::string_view chars = " \n\r")
    {
        return trimEnd(trimStart(str, chars), chars);
    }

    // Split string at one or more delimiters
    inline std::vector<std::string_view> split(std::string_view string, std::string_view delims = " ")
    {
        using namespace std;

        vector<string_view> tokens;
        for (size_t start = 0; start < string.size(); ) {
            // Find first delimiter
            const auto end = string.find_first_of(delims, start);

            // Add non-empty tokens
            if (start != end)
                tokens.emplace_back(string.substr(start, end-start));

            // Break at the end of string
            if (end == string_view::npos)
                break;

            start = end + 1;
        }

        return tokens;
    }

    // Split string at line breaks
    inline std::vector<std::string_view> splitLines(std::string_view string)
    {
        return split(string, "\r\n");
    }

    // Performs sprintf dynamically
    inline std::string format(const char* format, auto... args)
    {
        using namespace std;

        string buffer = string(2048, '\0');
        int len = snprintf(buffer.data(), 2048, format, args...);
        buffer.resize(len);
        if (len >= 2048) {
            len = snprintf(buffer.data(), len, format, args...);
        } else if (len < 0) {
            throw runtime_error(string("[Console] Failed to log message with format: '") + format + "'");
        }

        return buffer;
    }

    inline std::string replace(std::string_view input, std::string_view pattern, std::string_view replacement)
    {
        std::string str = std::string(input);
        size_t index = 0;
        while (index = str.find(pattern, index), index != std::string_view::npos)
        {
            str = str.replace(index, pattern.size(), replacement);
            index += replacement.size();
        }
        return str;
    }

}