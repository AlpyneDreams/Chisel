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

namespace engine
{
    inline struct Platform
    {
    #ifdef EDITOR
        static constexpr bool Editor  = true;
    #else
        static constexpr bool Editor  = false;
    #endif
        static constexpr bool Windows = PLATFORM_WINDOWS;
        static constexpr bool Linux   = PLATFORM_LINUX;

        // Returns empty string on cancel.
        std::string FilePicker(const char* startIn);

        // Starts in the directory of the last selected file.
        std::string FilePicker()
        {
            static std::string lastDir;
            std::string file = FilePicker(lastDir.empty() ? nullptr : lastDir.c_str());

            if (!file.empty())
                lastDir = file;

            return file;
        }
    } Platform;
}
