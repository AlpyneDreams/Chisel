
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
    static inline std::string ReadFGDFile(const char* path)
    {
        auto str = fs::readTextFile(path);
        if (!str) {
            Console.Warn("Could not read FGD file: {}", path);
            str = "";
        }
        return *str;
    }

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
                    case "baseclass"_h:
                    case "pointclass"_h:
                    case "solidclass"_h:
                    case "npcclass"_h:
                    case "keyframeclass"_h:
                    case "moveclass"_h:
                    case "filterclass"_h:
                        ParseClass();
                        continue;
                    case "include"_h:
                    {
                        cur++;
                        auto filename = Expect(Tokens.StringLiteral).text.str;
                        filename = str::trim(filename, "\"");
                        fs::Path dirname = fs::Path(fgd.path).dirname();
                        fs::Path path = dirname / filename;
                        auto str = ReadFGDFile(path);
                        Lexer lexer(str, false);
                        FGDParser parser(fgd, lexer.tokens);
                        parser.Parse();
                        break;
                    }
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
                    case "materialexclusion"_h:
                    {
                        cur++;
                        Expect('[');
                        while (*cur == Tokens.StringLiteral)
                        {
                            fgd.materialExclusion.push_back(*cur++);
                        }
                        Expect(']');
                        printdbg("@MaterialExclusion");
                        break;
                    }
                    case "autovisgroup"_h:
                    {
                        cur++;
                        Expect('=');
                        Expect(Tokens.StringLiteral);
                        Expect('[');
                        while (*cur == Tokens.StringLiteral)
                        {
                            cur++;
                            Expect('[');
                            while (*cur == Tokens.StringLiteral) cur++;
                            Expect(']');
                        }
                        Expect(']');
                        printdbg("@MaterialExclusion");
                        break;
                    }

                    default:
                        Console.Warn(name);
                }
            } while (*cur);
        }

        void ParseClass()
        {
            FGD::Class cls;

            auto type = Expect(Tokens.Name);
            while (*cur == Tokens.Name)
            {
                FGD::Helper& helper = cls.helpers.emplace_back();
                helper.name = *cur;
                cur++;

                // Parse helper args
                if (*cur == '(')
                {
                    cur++;
                    while (*cur != ')')
                    {
                        String& str = helper.params.emplace_back();
                        while (*cur != ',' && *cur != ')')
                        {
                            str += *cur++;
                        }
                        if (*cur == ',')
                            cur++;
                    }
                    Expect(')');
                }
                /*String str = "";
                for (auto& param : helper.params)
                    str += param + ", ";
                printdbg("Helper: {}({})", helper.name, str);*/
                printdbg("Helper: {}", helper.name);

                // Base classes
                if (str::toLower(helper.name) == "base")
                {
                    for (auto& param : helper.params)
                        if (fgd.classes.contains(param))
                            cls.bases.push_back(&fgd.classes.at(param));
                }
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

            fgd.classes.insert({ cls.name, cls });
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
            if (var.displayName.empty())
                var.displayName = var.name;
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
                if (*cur == Tokens.StringLiteral)
                    return ParseString();
                return *cur++;
            }
            return "";
        }


        String ParseString()
        {
            std::string part = Expect(Tokens.StringLiteral);
            String str = String(str::trim(part, "\""));
            while (*cur == '+') {
                cur++;
                part = Expect(Tokens.StringLiteral);
                str += String(str::trim(part, "\""));
            }
            return str;
        }

        int stoi(const Token& token)
        {
            return std::stoi(std::string(token.text.str));
        }
    };

    FGD::FGD(const char* path) : path(path)
    {
        auto str = ReadFGDFile(path);
        Lexer lexer(str, false);
        FGDParser parser(*this, lexer.tokens);
        parser.Parse();
    }
}
