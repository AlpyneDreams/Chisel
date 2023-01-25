#pragma once

#include "common/Common.h"
#include "Console.h"
#include "ConCommand.h"

namespace engine::commands
{
    inline ConCommand echo("echo", "Prints text to console", [](ConCmd& cmd) {
        Console.Log(cmd.args);
    });

    inline ConCommand error("error", "Prints error text to console", [](ConCmd& cmd) {
        Console.Error(cmd.args);
    });

    inline ConCommand warning("warning", "Prints warning text to console", [](ConCmd& cmd) {
        Console.Warn(cmd.args);
    });

    inline ConCommand help("help", "Prints information about a command", [](ConCmd& cmd) {
        if (cmd.argc > 0) {
            ConCommand::PrintHelp(cmd.argv[0]);
        } else {
            ConCommand::PrintCommandList();
        }
    });

    inline ConCommand clear("clear", "Clear console output", []() {
        Console.Clear();
    });

    inline ConCommand history("history", "Print command history", []() {
        for (auto& cmd : Console.GetHistory()) {
            Console.Log(cmd);
        }
    });
}

namespace engine::cvars
{
}

