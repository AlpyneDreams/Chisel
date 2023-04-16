
#include "chisel/Chisel.h"
#include "chisel/MapRender.h"
#include "common/String.h"
#include "formats/KeyValues.h"
#include "chisel/FGD/FGD.h"

#include "console/Console.h"
#include "gui/ConsoleWindow.h"
#include "gui/AssetPicker.h"
#include "gui/Layout.h"
#include "gui/Inspector.h"
#include "gui/Viewport.h"
#include "gui/Keybinds.h"

#include "common/Filesystem.h"
#include "render/Render.h"
#include "common/Parse.h"
#include "formats/Formats.h"

#include <cstring>
#include <vector>

namespace chisel
{
    void Chisel::Run()
    {
        Tools.Init();

        fgd = new FGD("core/test.fgd");

        // Add chisel systems...
        Renderer = &Tools.systems.AddSystem<MapRender>();
        Tools.systems.AddSystem<Keybinds>();
        Tools.systems.AddSystem<Layout>();
        console = &Tools.systems.AddSystem<GUI::ConsoleWindow>();
        Tools.systems.AddSystem<MainToolbar>();
        Tools.systems.AddSystem<SelectionModeToolbar>();
        Tools.systems.AddSystem<Inspector>();
        mainAssetPicker = &Tools.systems.AddSystem<AssetPicker>();
        Tools.systems.AddSystem<Viewport>();

        Tools.Loop();
        Tools.Shutdown();
    }

    Chisel::~Chisel()
    {
        delete fgd;
    }

    void Chisel::Save(std::string_view path)
    {
        if (path.ends_with("vmf"))
            ExportVMF(path, map);
        else if (path.ends_with("box"))
            ExportBox(path, map);
        else if (path.ends_with("map"))
            ExportMap(path, map);

        // TODO error handling
    }

    void Chisel::CloseMap()
    {
        Selection.Clear();
        map.Clear();
    }
    
    bool Chisel::LoadMap(std::string_view path)
    {
        if (path.ends_with("vmf"))
        {
            return ImportVMF(path, map);
        } else if (path.ends_with("box"))
        {
            return ImportBox(path, map);
        }

        return false;
    }

    void Chisel::CreateEntityGallery()
    {
        PointEntity* obsolete = map.AddPointEntity("obsolete");
        obsolete->origin = vec3(-8, -8, 0);

        vec3 origin = vec3(-7, -8, 0);
        for (auto& [name, cls] : fgd->classes)
        {
            if (cls.type == FGD::SolidClass || cls.type == FGD::BaseClass)
                continue;

            PointEntity* ent = map.AddPointEntity(name.c_str());
            ent->origin = origin * 128.f;
            if (++origin.x > 8)
            {
                origin.x = -8;
                origin.y++;
            }
        }
    }
}

namespace chisel::commands
{
    static ConCommand quit("quit", "Quit the application", []() {
        Tools.Shutdown();
        exit(0);
    });
    
    static ConCommand open_map("open_map", "Load a map from a file path.", [](ConCmd& cmd)
    {
        if (cmd.argc != 1)
            return Console.Error("Usage: open_map <path>");

        if (!Chisel.LoadMap(cmd.argv[0]))
            Console.Error("Failed to load map '{}'", cmd.argv[0]);
    });
}

int main(int argc, char* argv[])
{
    using namespace chisel;
    Chisel.Run();
}
