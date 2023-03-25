
#include "FGD.h"
#include "common/parser/Lexer.h"
#include "common/parser/Parser.h"

#include "console/Console.h"
#include "common/Filesystem.h"
#include "common/String.h"

template <>
struct fmt::formatter<chisel::Token> : fmt::ostream_formatter {};

//#define printdbg Console.Log
#define printdbg(...)

namespace chisel
{
    struct FGDParser : BaseParser
    {
        using String = FGD::String;

        FGD& fgd;
        FGDParser(FGD& fgd, std::vector<Token>& tokens) : BaseParser(tokens), fgd(fgd) {}

        void Parse()
        {
            do
            {
                if (*cur != '@') {
                    Console.Warn("Weird token: {}", *cur);
                    return;
                    continue;
                }

                auto name = str::toLower((++cur)->text);
                auto hash = HashedString(name);
                switch (hash)
                {
                    case "mapsize"_h:
                    {
                        cur++;
                        Expect('(');
                        auto min = Expect(Tokens.Number);
                        Expect(',');
                        auto max = Expect(Tokens.Number);
                        Expect(')');
                        printdbg("@MapSize ({}, {})", min, max);
                        fgd.minSize = stoi(min);
                        fgd.maxSize = stoi(max);
                        break;
                    }
                    case "baseclass"_h:
                    case "pointclass"_h:
                    case "solidclass"_h:
                    case "npcclass"_h:
                    case "keyframeclass"_h:
                    case "moveclass"_h:
                    case "filterclass"_h:
                        ParseClass();
                        continue;
                    default:
                        printdbg(name);
                }
            } while (*cur);
        }

        void ParseClass()
        {
            FGD::Class& cls = fgd.classes.emplace_back();

            auto type = Expect(Tokens.Name);
            while (*cur == Tokens.Name)
            {
                FGD::Helper& helper = cls.helpers.emplace_back();
                helper.name = *cur;
                cur++;
                if (*cur == '(')
                {
                    // TODO: Helper args
                    cur++;
                    while (*cur != ')') cur++;
                    Expect(')');
                }
                printdbg("Helper: {}", helper.name);
            }
            Expect('=');
            cls.name = Expect(Tokens.Name);
            cls.description = ParseDescription();

            printdbg("@{} = {} : {}", type, cls.name, cls.description);

            Expect('[');

            while(*cur != ']')
            {
                auto vartype = str::toLower(Expect(Tokens.Name).text);
                auto hash = HashedString(vartype);
                bool input = false;
                switch (hash)
                {
                    default:
                        cur--;
                        ParseVar(cls);
                        continue;
                    case "input"_h:
                        input = true;
                    case "output"_h:
                        ParseInputOutput(cls, input);
                        continue;
                    
                }
                cur++;
            }
            Expect(']');
        }

        void ParseVar(FGD::Class& cls)
        {
            auto name = Expect(Tokens.Name);
            FGD::Var& var = cls.variables.try_emplace(name).first->second;
            var.name = name;
            Expect('(');
            var.type = str::toLower(Expect(Tokens.Name).text);
            Expect(')');
            if (cur->text == "report"_h)
            {
                var.report = true;
                cur++;
            }
            if (cur->text == "readonly"_h)
            {
                var.readOnly = true;
                cur++;
            }
            var.displayName = ParseDescription();
            var.defaultValue = ParseDefault();
            var.description = ParseDescription();
            if (var.type == "choices" || var.type == "flags")
            {
                Expect('=');
                Expect('[');
                while (*cur != ']') cur++;
                Expect(']');
            }
            printdbg("    {}({})", var.name, var.type);
        }

        void ParseInputOutput(FGD::Class& cls, bool input)
        {
            auto name = Expect(Tokens.Name);
            FGD::InputOutput& io = (input ? cls.inputs : cls.outputs).try_emplace(name).first->second;
            io.name = name;
            Expect('(');
            io.type = Expect(Tokens.Name);
            Expect(')');
            io.description = ParseDescription();
            printdbg("    {} {}({}) : {}", input ? "input" : "output", io.name, io.type, io.description);
        }

        String ParseDescription()
        {
            if (*cur == ':') {
                cur++;
                return ParseString();
            }
            return "";
        }

        String ParseDefault()
        {
            if (*cur == ':') {
                cur++;
                if (*cur == ':')
                    return "";
                return *cur++;
            }
            return "";
        }


        String ParseString()
        {
            String str = Expect(Tokens.StringLiteral);
            while (*cur == '+') {
                cur++;
                str += Expect(Tokens.StringLiteral);
            }
            return str;
        }

        int stoi(const Token& token)
        {
            return std::stoi(std::string(token.text.str));
        }
    };

    FGD::FGD(const char* path)
    {
        auto str = fs::readTextFile(path);
        if (!str)
            return;
        auto tokens = Lexer(*str, false).tokens;
        FGDParser parser(*this, tokens);
        parser.Parse();
    }
}
