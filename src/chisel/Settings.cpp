#include "Settings.h"
#include "assets/Assets.h"
#include "common/Filesystem.h"
#include "console/Console.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

static constexpr const char* CONFIG_FILE = "chisel.json";

namespace chisel
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Settings, GamePath, SearchPaths);

    static Settings Instance;

    static struct DefaultSettings
    {
        DefaultSettings()
        {
            Settings::Reset();
        }
    } _;

    //-----------------------------------------------------------------------------
    // Default Settings
    //-----------------------------------------------------------------------------

    void Settings::Reset()
    {
        SearchPaths = {
            "$STEAMAPPS/Half-Life 2/hl2/hl2_textures",
            "$STEAMAPPS/Half-Life 2/hl2/hl2_misc",
            "$STEAMAPPS/Half-Life 2/hl1/hl1_pak",
            "$STEAMAPPS/Counter-Strike Source/cstrike/cstrike_pak",
        };
    }
    
    //-----------------------------------------------------------------------------

    void Settings::Apply()
    {
        Assets.ResetSearchPaths();
        for (auto& path : Settings::SearchPaths)
        {
            Assets.AddSearchPath(path);
        }

        Assets.Refresh();
    }

    //-----------------------------------------------------------------------------

    void Settings::Load()
    {
        if (fs::exists(CONFIG_FILE)) {
            // Load config file
            json j = json::parse(fs::readFile(CONFIG_FILE).value());
            j.get_to(Instance);
            Console.Log("[Settings] Read '{}'", CONFIG_FILE);

            Settings::Apply();
        } else {
            // Save defaults
            Save();
        }
    }

    //-----------------------------------------------------------------------------

    void Settings::Save()
    {
        Settings::Apply();

        json j = Instance;
        if (!fs::writeFile(CONFIG_FILE, j.dump(4)))
            Console.Error("[Settings] Failed to write '{}'", CONFIG_FILE);
        else
            Console.Log("[Settings] Wrote '{}'", CONFIG_FILE);
    }
}
