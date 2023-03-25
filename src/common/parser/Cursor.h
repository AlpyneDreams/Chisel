#pragma once

#include <span>
#include <string_view>
#include <type_traits>

#include "common/Common.h"
#include "Token.h"

namespace chisel
{
    template <class T>
    struct ViewType { using Type = std::span<T>; };

    template <>
    struct ViewType<char> { using Type = std::string_view; };

    template <class T = char, auto...>
    struct Cursor
    {
        using View = typename ViewType<T>::Type;
        View list;
        const T* pointer;

        Cursor(View list, const T* pointer) : list(list), pointer(pointer) {}
        Cursor(View list) : Cursor(list, &list.front()) { }

        operator T() const { return *pointer; }
        operator const T*() const { return pointer; }
        const T& operator *() { return *pointer; }
        const T* operator ->() const { return pointer; }
        const T& operator *() const { return *pointer; }

        //operator bool() const { return pointer < list.data() + list.size(); }

        Cursor operator+(int n) const {
            return Cursor(list, &pointer[n]);
        }

        Cursor operator-(int n) const {
            return Cursor(list, &pointer[-n]);
        }

        Cursor& operator+=(int n) {
            return *this = *this + n;
        }

        Cursor& operator-=(int n) {
            return *this = *this - n;
        }

        Cursor& operator++() {
            return *this += 1;
        }

        Cursor operator++(int)
        {
            Cursor tmp = *this;
            ++*this;
            return tmp;
        }

        Cursor& operator--() {
            return *this -= 1;
        }

        Cursor operator--(int)
        {
            Cursor tmp = *this;
            --*this;
            return tmp;
        }

        friend std::ostream& operator<<(std::ostream& os, const Cursor& c) {
            return os << *c;
        }

    };

    template <typename T>
    using BaseCursor = Cursor<T, true>;

    template <>
    struct Cursor<char> : BaseCursor<char>
    {
    };

    template <>
    struct Cursor<Token> : BaseCursor<Token>
    {
        Cursor(View list) : BaseCursor<Token>(list) {}

        bool operator==(TokenType t) const {
            return pointer->type == t;
        }

        friend std::ostream& operator<<(std::ostream& os, const Cursor<Token>& c) {
            //os << " " << c->location.line << " | ";
            //for (auto& token : c.list)
                //if (token.location.line == c->location.line)
                    //os << token << " ";
            return os; // << "\n";
        }

    };
}