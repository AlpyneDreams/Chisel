#pragma once
#include <string>
#include <vector>

namespace chisel
{
    struct Settings final
    {
        static void Reset();
        static void Load();
        static void Save();

    protected:
        static void Apply();
    public:
        static inline std::string GamePath;
        static inline std::vector<std::string> SearchPaths;
    };
}
