#pragma once

#include <cstdio>
#include <vector>

// TODO: fmtlib compile-time format strings?
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace engine
{
    namespace GUI { struct ConsoleWindow; }
    
    inline struct Console
    {
    public:
        enum class Level { Input = -1, Info = 0, Warning, Error };

        // TODO: Text styles for various log levels
        void Log(auto format = "", auto... args)      { Print(Level::Info, format, args...); }
        void Warning(auto format = "", auto... args)  { Print(Level::Warning, format, args...); }
        void Warn(auto format = "", auto... args)     { Warning(format, args...); }
        void Error(auto format = "", auto... args)    { Print(Level::Error, format, args...); }

        void Print(Level level, auto string = "") { Log("{}", string); }

        template <typename... Args> // MSVC doesn't like auto...
        void Print(Level level, const char* format = "", Args... args)
        {
            std::string str = Format(format, args...);
            log.push_back({level, str});
            if (level <= Level::Info)
                std::puts(str.c_str());
            else
                std::fprintf(stderr, "%s\n", str.c_str());
            newline = true;
        }

        void Printf(Level level, const char* format = "", auto... args)
        {
            std::string str = Format(format, args...);
            if (newline || log.size() <= 0) {
                log.push_back({Level::Info, str});
                newline = false;
            } else {
                log.back().text += str;
            }
            if (level <= Level::Info)
                std::printf("%s", str.c_str());
            else
                std::fprintf(stderr, "%s", str.c_str());
        }

        template <typename... Args> // MSVC doesn't like auto...
        inline std::string Format(const char* format = "", Args... args)
        try {
            return fmt::format(fmt::runtime(format), args...);
        } catch (fmt::format_error const& err) {
            Error("Cannot format log message: {}", err.what());
            return format;
        }
        
        void Clear()
        {
            log.resize(0);
            newline = true;
        }

        const std::vector<std::string>& GetHistory() const { return history; }

        void Execute(const char* string);
    protected:
        struct Entry { Level level; std::string text; };
        std::vector<Entry> log;
        std::vector<std::string> history;
        bool newline = true;

        friend struct GUI::ConsoleWindow;
        friend struct ConCommand;
    } Console;
}
