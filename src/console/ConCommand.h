#pragma once

#include "common/Common.h"
#include "common/String.h"

#include "Console.h"

#include <map>
#include <span>
#include <functional>
#include <stdexcept>
#include <string_view>

namespace engine
{
    struct ConCommand;

    /** Represents console command arguments */
    struct ConCmd
    {
        ConCommand* command;
        const char* args = "";
        uint argc = 0;
        std::span<std::string_view> argv;

        ConCmd() {}
        ConCmd(ConCommand* command) : command(command) {}
        ConCmd(ConCommand* command, std::span<std::string_view>&& arguments)
          : command(command),
            args(arguments[0].data()),
            argc(arguments.size()),
            argv(arguments)
        {}

        constexpr auto begin() const { return argv.begin(); }
        constexpr auto end() const { return argv.end(); }
    };

    /** ConCommand callback function */
    using ConFunc = std::function<void(ConCmd&)>;

    /** Defines a unique console command */
    struct ConCommand
    {
    protected:
        const char* name;
        const char* description = "";
        ConFunc function;

    public:
        ConCommand(const char* name, const char* description, std::function<void(ConCmd&)> func, auto... flags)
          : name(name),
            description(description),
            function(func)
        {
            if (commands.contains(name)) {
                throw std::runtime_error(std::string("Duplicate registered console command: '") + name + "'");
            }
            commands[name] = this;
        }

        // Supports using a void() function
        ConCommand(const char* name, const char* description, std::function<void()> func, auto... flags)
          : ConCommand(name, description, [func](ConCmd&) { func(); }, flags...)
        {}

        virtual ~ConCommand()
        {
            commands.erase(name);
        }

        void operator()(auto... args) { return this->Invoke(args...); }

        void Invoke()
        {
            ConCmd cmd(this);
            Invoke(cmd);
        }

        void Invoke(auto... args)
        {
            auto arglist = std::vector<std::string_view> { args... };
            ConCmd cmd = ConCmd(this, std::span<std::string_view>(arglist.begin(), arglist.end()));

            Invoke(cmd);
        }

        virtual void Invoke(ConCmd& cmd)
        {
            if (function)
                function(cmd);
        }

        virtual void PrintHelp()
        {
            Console.Log(name);
            Console.Log("- {}", description);
        }

    public:

        static void Execute(const char* string)
        {
            Console.history.push_back(string);
            Console.Print(Console::Level::Input, "> {}", string);

            auto tokens  = str::split(string);
            if (tokens.size() == 0) {
                return;
            }

            auto name = tokens[0];
            if (!commands.contains(name)) {
                Console.Log("Unrecognized command: '{}'", name.data());
                return;
            }

            auto command = commands[name];

            ConCmd cmd;
            if (tokens.size() > 1) {
                cmd = ConCmd(command, std::span<std::string_view>(tokens.begin() + 1, tokens.end()));
            } else {
                cmd = ConCmd(command);
            }

            command->Invoke(cmd);
        }

        static void PrintHelp(std::string_view& name)
        {
            if (commands.contains(name)) {
                commands[name]->PrintHelp();
            } else {
                Console.Warn("help: no command or variable named '{}'", name);
            }
        }

        static void PrintCommandList()
        {
            Console.Log("Available commands: ");
            for (auto& [name, cmd] : commands) {
                Console.Log("- {}: {}", cmd->name, cmd->description);
            }
        }

    private:
        static inline std::map<std::string_view, ConCommand*> commands;
    };

    inline void Console::Execute(const char *string) { ConCommand::Execute(string); }
}
