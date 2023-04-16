#pragma once

// Avoid using #if or #ifdef when possible.
#ifdef _WIN32
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_LINUX   0
    #define PLATFORM_X11     0
#else
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX   1
    #define PLATFORM_X11     1
#endif

#include <string>
#include <span>

namespace chisel
{
    struct ExtensionName
    {
        std::string_view prettyName;
        std::string_view ext;
    };

    inline struct Platform
    {
        static constexpr bool Windows = PLATFORM_WINDOWS;
        static constexpr bool Linux   = PLATFORM_LINUX;

        // Returns empty string on cancel.
        std::string FilePicker(bool open, std::span<const ExtensionName> extensions, const char* startIn);

        // Starts in the directory of the last selected file.
        std::string FilePicker(bool open, std::span<const ExtensionName> extensions)
        {
            static std::string lastDir;
            std::string file = FilePicker(open, extensions, lastDir.empty() ? nullptr : lastDir.c_str());

            if (!file.empty())
                lastDir = file;

            return file;
        }
    } Platform;
}
