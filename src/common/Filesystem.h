#pragma once

#include "Common.h"
#include "Path.h"
#include "Span.h"

#include <fstream>
#include <iterator>
#include <type_traits>
#include <string>
#include <filesystem>
#include <optional>

namespace chisel::fs
{
    using std::filesystem::exists;
    using std::filesystem::copy;

    using enum std::filesystem::copy_options;
    
    // Copy file. Overwrites existing file by default.
    inline bool copyFile(const Path& from, const Path& to, bool overwrite = true)
    {
        std::error_code ec;
        std::filesystem::copy_file(from, to, overwrite ? overwrite_existing : skip_existing, ec);
        return !ec;
    }

    // Read file into buffer.
    template <typename T = Buffer, bool Binary = !std::is_same_v<T, std::string>>
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

    // Read text file.
    inline std::optional<std::string> readTextFile(const Path& path)
    {
        return readFile<std::string>(path);
    }
}
