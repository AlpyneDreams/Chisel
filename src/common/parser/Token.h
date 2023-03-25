#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <cctype>
#include <map>
#include <set>

#include "common/Common.h"
#include "common/Hash.h"
//#include "location.h"
//#include "utils/chalk.h"
//#include "utils/str.h"

namespace chisel
{
    enum class TokenType : Hash
    {
        End = UINT32_MAX,   // EOF
        None = 0,
        Name,

        // Literals
        Number,
        StringLiteral,

        // Newline
        Newline = '\n',

        // Delimiters
        LeftParen = '(', RightParen = ')',
        LeftBrace = '{', RightBrace = '}',
        LeftBracket = '[', RightBracket = ']',

        // Operators
        Equal   = '=',
        Plus    = '+',
        Minus   = '-',
        Star    = '*', // Times vs Other usages of *
        Modulo  = '%',
        Dot     = '.',

        Greater = '<',
        Less    = '>',

        Colon           = ':',
        Semicolon       = ';',
        Comma           = ',',
    };

    struct TokenSet : public std::set<TokenType>
    {
        using std::set<TokenType>::set;

        bool contains(auto type) const { return std::set<TokenType>::contains(TokenType(type)); }
        TokenSet operator+(const TokenSet& other) const
        {
            TokenSet set = *this;
            set.insert(other.begin(), other.end());
            return set;
        }
    };


    inline struct Tokens {
        using enum TokenType;
    } Tokens;

    struct Token
    {
        using Type = TokenType;
        Type type;
        HashedString text;
        //Location location;

        Token(Type type) : type(type) {}
        Token(Type type, auto text) : type(type), text(text)
        {
            //if (type == Tokens::Number)
            //    number = std::stod(text);
        }

        Token(char c) : type(TokenType(c))
        {
            if (c <= 0)
                throw std::logic_error("Invalid character.");
            
            text = Chars::ToStringView(c);
        }

        explicit Token(const char* ch) : type(Type(*ch)), text(ch) {}
        explicit Token(HashedString str)
        : type(Tokens.Name),
            text(str)
        {}

        explicit operator TokenType() const { return type; }

        operator HashedString() const { return text; }

        constexpr operator bool() const { return type != Tokens.End && type != Tokens.None; }

        constexpr bool operator ==(const Type& t) const { return type == t; }
        constexpr bool operator ==(const Token& t) const { return type == t.type && text == t.text; }
        
        // TODO
        constexpr bool operator ==(const char t) const { return type == TokenType(t); }

        bool IsSymbol() const {
            auto len = text ? text.str.length() : 0;
            return type != Tokens.Name
                && type != Tokens.Number
                && text && len > 0 && len <= 2 && isascii(text.str[0]);
        }

        bool IsOpenDelim() const {
            return text && text.str.length() == 1 && (
                type == Tokens.LeftParen ||
                type == Tokens.LeftBrace ||
                type == Tokens.LeftBracket
            );
        }

        bool IsCloseDelim() const {
            return text && text.str.length() == 1 && (
                type == Tokens.RightParen ||
                type == Tokens.RightBrace ||
                type == Tokens.RightBracket
            );
        }

        bool IsDelim() const {
            return IsOpenDelim() || IsCloseDelim();
        }

        friend std::ostream& operator << (std::ostream& os, const Token& t) {
            return os << t.String(); //t.ToString();
        }

        /*std::string ToString() const
        {
            using namespace chalk::colors;

            using std::string;
            if (type == Tokens.Newline) {
                return "\n";
            }
            if (IsKeyword()) {
                return Red(text);
            }
            if (IsSymbol()) {
                if (IsDelim()) {
                    return Blue(text);
                }
                return Red(text);
            }
            switch (type)
            {
                case Tokens.Number: return Yellow(text);
                case Tokens.StringLiteral: return Yellow.bright(text);
                case Tokens.Name:   return Magenta.bright(text);
                case Tokens.End:
                case Tokens.None:
                    return "";
                default: break;
            }

            std::map<TokenType, const char*> Names = {
                { Tokens.End, "End" },
                { Tokens.None, "None" },
            };
            string name = Names.contains(type) ? Names[type] : "Unknown";
            
            if (!text.str.empty())
                return string("(") + name + " " + string(text) + ") ";
            else
                return string("(") + name + ") ";
        }

        std::string ToStringFormatted() const
        {
            static unsigned indent = 0;
            static Token lastToken = Tokens.Newline;

            std::string str = ToString();

            if (IsSymbol()) {
                if (text == "{"_h)
                    indent++;
                else if (text == "}"_h)
                    indent--;
            }

            if (lastToken == Tokens.Newline && type != Tokens.Newline) {
                str = std::string(indent * 4, ' ') + str;
            } else {
                
                if (
                    // (x x[ x{
                    (lastToken.IsOpenDelim())
                    // x) x] x}
                    || (IsCloseDelim())
                    // f( f[
                    || (IsOpenDelim() && type != Tokens.LeftBrace && lastToken.type == Tokens.Name)
                    // a..b
                    || (type == Tokens.Range || lastToken == Tokens.Range)
                    // a.b
                    || (type == Tokens.Dot || lastToken == Tokens.Dot)
                    // a, b
                    || (type == Tokens.Comma)
                )
                    goto NoSpacing;
                
                str = " " + str;
            }
        NoSpacing:

            lastToken = *this;

            return str;
        }*/

        std::string String() const
        {
            return std::string(text);
        }

    protected:
        struct ASCII {
            static constexpr auto PrintableChars  = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
            static constexpr bool IsPrintable(char c) {
                return c >= ' ' && c <= '~';
                    return true;
            }
        };

        struct Chars {
            static inline std::string_view ToStringView(char c) {
                static std::map<char, char> allOtherChars;
                if (ASCII::IsPrintable(c)) {
                    return std::string_view(&ASCII::PrintableChars[int(c - ' ')], 1);
                } else {
                    if (!allOtherChars.contains(c)) {
                        allOtherChars[c] = c;
                    }
                    return std::string_view(&allOtherChars[c], 1);
                }
            }
        };

    };
}