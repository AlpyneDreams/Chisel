#pragma once

#include <filesystem>

namespace chisel::fs
{
    // Wrapper for std::filesystem::path that maintains a UTF-8 string copy.
    class Path
    {
        std::filesystem::path   m_path;
        std::string             m_text;

    public:
        Path(auto... args) : m_path(args...), m_text(m_path.string()) {}

        Path dirname() const { return m_path.parent_path(); }
        Path ext() const { return m_path.extension(); }

        Path& setExt(auto ext) { m_path.replace_extension(ext); m_text = m_path.string(); return *this; }

        // Append a subpath with a separator
        Path& operator /=(const Path& path)
        {
            m_path /= path.m_path;
            m_text = m_path.string();
            return *this;
        }

        // Append a string with no separator
        Path& operator +=(const auto& str)
        {
            m_path += str;
            m_text += str;
            return *this;
        }

        // Conversion to const char*
        operator const char*() const { return m_text.c_str(); }

        // Conversion to std::filesystem::path
        operator const std::filesystem::path&() const { return m_path; }

        // Conversion to std::string_view
        operator std::string_view() const { return m_text; }

        friend Path operator /(const Path& lhs, const Path& rhs)
        {
            Path temp = lhs;
            return (temp /= rhs);
        }

        friend Path operator +(const Path& lhs, const auto& rhs)
        {
            Path temp = lhs;
            return (temp += rhs);
        }

        friend std::ostream& operator<<(std::ostream& os, const Path& path) {
            return os << path.m_text;
        }
        
        friend struct std::hash<chisel::fs::Path>;

        bool operator ==(const Path& path) const { return m_path == path.m_path; }
    };
}

template<>
struct std::hash<chisel::fs::Path>
{
    std::size_t operator()(const chisel::fs::Path& p) const
    {
        return std::hash<std::filesystem::path>()(p.m_path);
    }
};
