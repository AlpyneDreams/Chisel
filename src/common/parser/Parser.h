#pragma once

#include "common/Hash.h"
#include "Token.h"
#include "Cursor.h"
//#include "utils/chalk.h"
//#include "errs.h"

#include <vector>
#include <iostream>

namespace chisel
{
    /*  The base parser vists tokens in a list using a cursor, and
        mainly provides helper functions for parsing and analysis. */
    struct BaseParser
    {
        std::vector<Token> tokens;
        Cursor<Token> cur;

        BaseParser(std::vector<Token>& tokens) : tokens(tokens), cur(tokens) {}

        Token Next() { return ++cur; }

        Token Expect(const Token& prototype)
        {
            if (cur->type != prototype.type) {
                // TODO: Exception?
                std::cerr << "err"/*cur->location*/ << ": Unexpected token '" << *cur << "'.\n";
                std::cerr << "err"/*cur->location*/ << ": Expected '" << prototype << "'.\n";
                std::cerr << cur;
                exit(1);
            }
            return cur++;
        }

        Token Expect(auto... tokens)
        {
            TokenSet set { tokens... };
            if (!set.contains(cur->type)) {
                // TODO: Exception?
                std::cerr << "err"/*cur->location*/ << ": Unexpected token '" <<  *cur << "'.\n";
                // TODO: Expected...
                std::cerr << cur;
                exit(1);
            }
            return cur++;
        }

        int Skip(auto... args) { return Skip(TokenSet { args... }); }

        int Skip(const TokenSet& set)
        {
            int count = 0;
            while (set.contains(cur->type)) {
                cur++;
                count++;
            }
            return count;
        }

    };
}
