
#include "platform/Platform.h"
#include "common/String.h"
#include "console/Console.h"

#include <stdio.h>
#include <string>

// Linux Platform-specific implementations
// (and probably some other Unices)

namespace engine
{
    std::string Platform::FilePicker(const char* startIn)
    {
        std::string command = "zenity --file-selection";
        if (startIn)
            command = std::string("zenity --file-selection --filename=\"") + startIn + "\"";

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
