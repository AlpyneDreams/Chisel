#pragma once

#include "Common.h"
#include "Path.h"

#include <fstream>
#include <iterator>
#include <type_traits>
#include <string>
#include <tuple>
#include <filesystem>
#include <ostream>
#include <optional>
#include <vector>
#include <cstdint>

#include <fmt/ostream.h>

// Automatic path to string conversion
namespace std::filesystem {
    std::ostream& operator<<(std::ostream& os, const path& path);
    std::string operator+(const std::string& str, const path& path);
}

namespace chisel::fs
{
    using std::filesystem::exists;

    template <typename T = std::vector<byte>, bool Binary = !std::is_same_v<T, std::string>>
    inline std::optional<T> readFile(const Path& path)
    {
        using namespace std;
        std::ios::openmode flags = ios::in;
        if constexpr (Binary)
            flags |= ios::binary;

        ifstream file(std::filesystem::path(path), flags);
        if (!file || file.bad())
            return std::nullopt;

        T buffer;

        // Resize buffer to file size
        file.seekg(0, ios::end);
        auto size = file.tellg();
        buffer.resize(size);
        file.seekg(0, ios::beg);

        // Read file into buffer
        file.read((char*)&buffer[0], size);

        return buffer;
    }

    inline std::optional<std::string> readTextFile(const Path& path)
    {
        return readFile<std::string>(path);
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

template <> struct fmt::formatter<std::filesystem::path> : ostream_formatter {};
template <> struct fmt::formatter<chisel::fs::Path> : ostream_formatter {};