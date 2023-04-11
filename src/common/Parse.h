#pragma once

#include "common/SmallVector.h"
#include "common/Result.h"

#include <charconv>
#include <string.h>

namespace chisel::stream
{
    template <typename T>
    Result<T> Parse(const char*& first, const char* end)
    {
        T obj;
        auto result = std::from_chars(first, end, obj);
        if (result.ec == std::errc{})
        {
            first = result.ptr;
            return Result<T>::Success(obj); 
        }

        return Result<T>::Error(BasicErrorCode::NotFound);
    }

    template <typename T>
    Result<T> Parse(const char*& first, size_t length)
    {
        return Parse<T>(first, first + length);
    }

    template <typename T>
    Result<T> Parse(StringView view)
    {
        return Parse<T>(view.data, view.size);
    }

    template <typename T>
    Result<T> Parse(const char*& first)
    {
        return Parse<T>(first, first + strlen(first));
    }

    inline bool EndOfStream(const char* first, const char* end)
    {
        return first == end || *first == '\0';
    }

    inline bool CharMatches(char character, StringView delims)
    {
        for (auto delim : delims)
        {
            if (character == delim)
                return true;
        }

        return false;
    }

    inline size_t Consume(const char*& first, const char* end, StringView delims)
    {
        size_t count = 0;
        while (!EndOfStream(first, end) && CharMatches(*first, delims))
        {
            first++;
            count++;
        }

        return count;
    }

    static constexpr const char* WhitespaceDelimiters = " \t";
    static constexpr const char* NewlineDelimiters = "\r\n";
    static constexpr const char* WhitespaceOrNewlineDelimiters = " \t\r\n";

    inline size_t ConsumeSpace(const char*& first, const char* end)
    {
        return Consume(first, end, WhitespaceDelimiters);
    }

    inline size_t ConsumeSpaceAndNewLine(const char*& first, const char* end)
    {
        return Consume(first, end, WhitespaceOrNewlineDelimiters);
    }

    inline bool IsWhitespace(char val)
    {
        return val == ' ' || val == '\t';
    }

    inline bool IsNewLine(char val)
    {
        return val == '\r' || val == '\n';
    }

    template <typename OutArray = SmallVector<char, 1>>
    size_t ReadOrAdvance(const char*& first, const char* end, StringView delims, size_t advancement = 0, OutArray* array = nullptr)
    {
        size_t count = 0;
        while (!EndOfStream(first, end) && !CharMatches(*first, delims))
        {
            if (array)
                array->push_back(*first);
            first++;
            count++;
        }

        if (EndOfStream(first, end))
            return count;

        first += advancement;
        
        return count;
    }

    template <typename OutArray = SmallVector<char, 1>>
    size_t ReadString(const char*& first, const char* end, StringView delims, OutArray& array)
    {
        return ReadOrAdvance(first, end, delims, 0, &array);
    }

    inline size_t AdvancePast(const char*& first, const char* end, StringView delims)
    {
        return ReadOrAdvance(first, end, delims, 1);
    }

    inline bool IsStringToken(StringView fullRange, const char* first)
    {
        if (*first != '"')
            return false;

        // Make sure we are in-range for the checks below.
        if (fullRange.begin() == first || fullRange.begin() == first - 1)
            return true;

        // Handle \\"
        if (*(first - 2) == '\\')
            return true;

        // Hanle \"
        if (*(first - 1) == '\\')
            return false;

        return true;
    }

    inline bool IsCPPComment(StringView fullRange, const char* first)
    {
        if (*first != '/')
            return false;

        if (fullRange.end() == first + 1)
            return false;

        if (*(first + 1) != '/')
            return false;

        return true;
    }
}
