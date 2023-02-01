#pragma once

#include "Common.h"
#include <fstream>
#include <iterator>
#include <stdexcept>
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
    using Path = std::filesystem::path;

    using std::filesystem::exists;

    template <typename T>
    inline std::optional<T> readFileImpl(const Path& path, bool binary)
    {
        using namespace std;
        std::ios::openmode flags = ios::in;
        if (binary) flags |= ios::binary;

        ifstream file(path, flags);
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

    inline std::optional<std::string> readFileText(const Path& path)
    {
        return readFileImpl<std::string>(path, false);
    }

    inline std::optional<std::vector<uint8_t>> readFileBinary(const Path& path)
    {
        return readFileImpl<std::vector<uint8_t>>(path, true);
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