#pragma once

#include <filesystem>

namespace chisel::fs
{
    class Path
    {
        using Char = char;
        using String = std::string;
        using StringView = std::string_view;

        struct PathNode;

        String text;
        std::vector<PathNode> parts;

        static constexpr StringView delims = "\\/";

    public:
        Path()                  = default;
        Path(const Path& p)     = default;
        //Path(Path&& p)          = default;
        ~Path()                 = default;

        Path(const auto& source)    { Assign(source); }
        Path(String&& source)       { Assign(StringView(source)); }
        Path(auto first, auto last) { Assign(StringView(first, last)); }

        // Append a subpath with a separator
        Path& operator /=(const Path& path) { Append(path); return *this; }

        // Conversion to const char*
        operator const Char*() const { return text.data(); }

        // Conversion with std::filesystem::path
        Path(const std::filesystem::path& path) { Assign(StringView(path.string())); }
        operator std::filesystem::path() const { return std::filesystem::path(text); }

        friend Path operator /(const Path& lhs, const Path& rhs)
        {
            Path temp = lhs;
            return (temp /= rhs);
        }
        

    protected:
        void Assign(StringView string)
        {
            text = string;
            using namespace std;

            for (size_t start = 0; start < string.size(); ) {
                // Find first delimiter
                const auto end = string.find_first_of(delims, start);

                // Add non-empty tokens
                if (start != end)
                    parts.emplace_back(start, string.substr(start, end-start).size());

                // Break at the end of string
                if (end == string_view::npos)
                    break;

                start = end + 1;
            }

            // TODO: Normalize text here...
        }

        void Append(const Path& path)
        {
            size_t offset = text.size();

            for (auto& part : path.parts)
            {
                text += '/';
                text += part(path.text);
                parts.push_back(part + offset);
            }
        }

    private:
        struct PathNode
        {
            size_t start, length;
            
            StringView operator()(const String& str) const {
                return StringView(str.data() + start, length);
            }

            PathNode operator+(size_t amt) const {
                return PathNode(start + amt, length);
            }
        };
    };
}