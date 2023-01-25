#pragma once

#include "Common.h"
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>
#include <filesystem>
#include <ostream>

#include <fmt/ostream.h>

// Automatic path to string conversion
namespace std::filesystem {
    std::ostream& operator<<(std::ostream& os, const path& path);
    std::string operator+(const std::string& str, const path& path);
}

namespace chisel::fs
{
    using Path = std::filesystem::path;

    using std::filesystem::exists;

    inline const std::string readFile(const Path& path)
    {
        using namespace std;
        ifstream file(path, ios::in | ios::binary);
        if (!file)
            throw runtime_error(std::string("[FS] Failed to open file: ") + path);

        std::string buffer;

        // Resize buffer to file size
        file.seekg(0, ios::end);
        buffer.resize(file.tellg());
        file.seekg(0, ios::beg);

        // Read file into buffer
        file.read(&buffer[0], buffer.size());

        return buffer;
    }
}

namespace std::filesystem
{
    // Write path to ostream
    inline std::ostream& operator<<(std::ostream& os, const path& path) {
        return os << path.string();
    }

    // Append path to string
    inline std::string operator+(const std::string& str, const path& path) {
        return str + path.string();
    }
}

template <> struct fmt::formatter<chisel::fs::Path> : ostream_formatter {};