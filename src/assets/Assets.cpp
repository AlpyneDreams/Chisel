#include "Assets.h"
#include "assets/SearchPaths.h"

#include <variant>
#include <vector>

namespace chisel
{
    static bool Quiet = false;

    Assets::Assets()
    {
        Quiet = true;
        ResetSearchPaths();
        Quiet = false;
    }

    Assets::~Assets()
    {
        // Delete all remaining assets on the heap
        while (Asset::AssetDB.size() > 0)
        {
            auto& [path, asset] = *Asset::AssetDB.begin();
            uint32 refCount = (asset->incRef(), asset->decRef());
            Console.Warn("Deleted unreleased asset: (references = {}) '{}'", refCount, path);
            delete asset;
        }
    }

    //=============================================================================
    // Asset Loading
    //=============================================================================

    static inline std::string NormalizePath(const fs::Path& path)
    {
        std::string str = str::toLower(path);
        str = str::replace(str, "\\", "/");
        return str;
    }

    bool Assets::IsLoaded(const Path& path)
    {
        return Asset::AssetDB.contains(path);
    }

    bool Assets::FileExists(const Path& path)
    {
        // Check loose files
        for (const auto& dir : searchPaths)
        {
            Path fullPath = dir / path;
            if (fs::exists(fullPath))
                return true;
        }

        // Check paks
        std::string cleanPath = NormalizePath(path);
        for (const auto& pak : pakFiles)
        {
            if (pak->files().contains(cleanPath))
                return true;
        }

        return false;
    }

    std::optional<Buffer> Assets::ReadFile(const Path& path, bool complain)
    {
        auto loose_data = ReadLooseFile(path);
        if (loose_data)
            return loose_data;

        auto vpk_data = ReadPakFile(path);
        if (vpk_data)
            return vpk_data;

        if (complain)
            Console.Error("[Assets] Can't find file: '{}'", path);
        return std::nullopt;
    }

    std::optional<Buffer> BaseAssetLoader::ReadFile(const fs::Path& path, bool complain)
    {
        return Assets.ReadFile(path, complain);
    }

    std::optional<Buffer> Assets::ReadLooseFile(const Path& path)
    {
        for (const auto& dir : searchPaths)
        {
            Path fullPath = dir / path;
            if (fs::exists(fullPath))
                return fs::readFile(fullPath);
        }
        return std::nullopt;
    }

    std::optional<Buffer> Assets::ReadPakFile(const Path& path)
    {
        std::string cleanPath = NormalizePath(path);

        for (const auto& pak : pakFiles)
        {
            auto file = pak->file(cleanPath);
            if (!file)
                continue;

            auto stream = libvpk::VPKFileStream(*file);

            Buffer data;
            data.resize(file->length());
            stream.read((char*)data.data(), file->length());

            return data;
        }
        return std::nullopt;
    }


    //=============================================================================
    // Search Paths
    //=============================================================================

    void Assets::AddSearchPath(const Path& p)
    {
        Path path = SearchPaths.Resolve(p);
        if (!fs::exists(path)) {
            auto vpk = path + "_dir.vpk";
            if (!fs::exists(vpk)) {
                return Console.Error("[Assets] Failed to find search path '{}'", path);
            } else {
                path = vpk;
            }
        }

        if (!std::filesystem::is_directory(p))
        {
            if (str::toLower(path.ext()) == ".vpk")
                return AddPakFile(path);
            else
                return Console.Error("[Assets] Search path '{}' is not a directory", path);
        }

        searchPaths.push_back(path);
        if (!Quiet) Console.Log("[Assets] Added search path: '{}'", p);
    }

    void Assets::AddPakFile(const Path& p)
    {
        Path path = SearchPaths.Resolve(p);
        try
        {
            auto pak = std::make_unique<libvpk::VPKSet>(path);
            pakFiles.emplace_back(std::move(pak));
            if (!Quiet) Console.Log("[Assets] Loaded pak file: '{}'", p);
        }
        catch (const std::exception& e)
        {
            Console.Error("[Assets] Failed to load pak file '{}': {}", p, e.what());
        }
    }

    void Assets::ResetSearchPaths()
    {
        searchPaths.clear();
        pakFiles.clear();
        AddSearchPath("core");
    }

    void Assets::Refresh()
    {
        Console.Log("[Assets] Refreshing...");
        OnRefresh();
    }
}
