
#include "platform/Platform.h"
#include "common/String.h"
#include "console/Console.h"

#include <stdio.h>
#include <string>
#include <sstream>

// Linux Platform-specific implementations
// (and probably some other Unices)

namespace chisel
{
    std::string Platform::FilePicker(bool open, std::span<const ExtensionName> extensions, const char* startIn)
    {
        std::stringstream command_stream;
        command_stream << "zenity --file-selection";
        if (startIn)
        {
            command_stream << " --filename=\"" << startIn << "\"";
        }
        if (!open)
        {
            command_stream << " --save";
        }
        for (const auto& extension : extensions)
        {
            command_stream << " --file-filter='";
            command_stream << extension.prettyName << " |";
            auto types = str::split(extension.ext, ",");
            for (const auto& type : types)
            {
                std::string lowerType = std::string(type);
                std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), [](unsigned char c) { return std::tolower(c); });
                command_stream << " " << lowerType;
            }
            command_stream << "'";
        }

        std::string command = command_stream.str();

        std::string filename;
        filename.resize(1024);

        FILE* f = popen(command.c_str(), "r");
        fgets(filename.data(), filename.size(), f);

        int ret = pclose(f);
        if (ret < 0)
            Console.Error("FilePicker(): failed (code: {})", ret);

        // Strip DLE if any
        auto pos = filename.find_last_of(10);
        if (pos != std::string::npos)
            filename[pos] = 0;

        // Truncate to first null
        return std::string(filename.data());
    }
}
