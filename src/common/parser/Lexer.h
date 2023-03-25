#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <cctype>

#include "common/Common.h"
//#include "location.h"
#include "Token.h"
#include "Cursor.h"
//#include "utils/str.h"
//#include "errs.h"

namespace chisel
{
    struct Lexer
    {
        using String = std::string;
        String& text;
        Cursor<char> cur;
        //Location location = { "input", 1, 0 };
        std::vector<Token> tokens;

        Lexer(String& input) : text(input), cur(std::string_view(text))
        {
            for (Token t = GetToken(); t; t = GetToken()) {
                //if (!t.location.line)
                    //t.location = location;
                tokens.push_back(t);
            }
            tokens.push_back(Tokens.End);
        }

        Token Newline()
        {
            Token t = Tokens.Newline;
            //t.location = location;
            //location.line++;
            //location.col = location.col ? 1 : 0;
            return t;
        }

        Token GetToken()
        {
            // Skip whitespace
            while (isspace(cur))
            {
                char last = cur++;
                if (isnewline(last) && tokens.size() > 0 && tokens.back() != Tokens.Newline)
                    return Newline();
            }

            // Name: [a-zA-Z_][a-zA-Z0-9_]*
            if (isalpha(cur) || cur == '_')
            {
                const char* start = cur;
                const char* end = cur;

                while (isalnum(++cur) || cur == '_')
                    end = cur;
                
                HashedString name = HashedString(start, end - start + 1);
                return Token(name);
            }

            // Number: [0-9.]+
            if (isdigit(cur) || (cur == '.' && isdigit(cur+1)))
            {
                const char* start = cur;

                do {
                    if (cur == '.' && cur-1 == '.') {
                        cur--;
                        break;
                    }
                    cur++;
                } while (isdigit(cur) || cur == '.');
                
                return Token(Tokens.Number, HashedString(start, cur - start));
            }

            // Comment: #.*|//.*
            if (cur == '#' || (cur == '/' && cur+1 == '/'))
            {
                // Until end of line
                do
                    cur++;
                while (!isnewline(cur) && cur);

                // Include newlines from trailing comments only
                if (isnewline(cur) && tokens.size() > 0 && tokens.back() != Tokens.Newline)
                    return Newline();

                // Get next token
                if (++cur)
                    return GetToken();
            }

            // String Literals: ".*" | '.*'
            if (cur == '"' || cur == '\'')
            {
                const char* start = cur;
                char delim = *cur;

                do {
                    cur++;
                } while (cur && *cur != delim);
                cur++;

                return Token(Tokens.StringLiteral, HashedString(start, cur - start));
            }
            
            if (!cur) {
                return Tokens.End;
            }

            // TODO: Digraphs
            // Need to not count stuff starting with '.' as numbers
            /*if (issymbol(cur) && issymbol(cur+1))
            {
                Token t = Token(HashedString(cur, 2));
                if (Tokens.Digraphs.contains(t.type)) {
                    cur += 2;
                    return t;
                }

            }*/
            
            const char* last = cur++;

            // Operator or unknown
            return Token(last);
        }

    protected:
        static bool isnewline(char c) {
            return (c == '\n' || c == '\r');
        }

        static bool issymbol(char c) {
            return !isspace(c) && !isalnum(c);
        }
    };
}
