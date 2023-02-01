
#include "chisel/Chisel.h"
#include "chisel/MapRender.h"
#include "common/String.h"
#include "chisel/VMF/KeyValues.h"
#include "chisel/VMF/VMF.h"

#include "console/Console.h"
#include "gui/ConsoleWindow.h"
#include "gui/Layout.h"
#include "gui/Viewport.h"
#include "gui/Keybinds.h"

#include "common/Filesystem.h"
#include "render/Render.h"

#include <cstring>
#include <vector>

namespace chisel
{
    void Chisel::Run()
    {
        Tools.Init();

        // Add chisel systems...
        Renderer = &Tools.systems.AddSystem<MapRender>();
        Tools.systems.AddSystem<Keybinds>();
        Tools.systems.AddSystem<Layout>();
        Tools.systems.AddSystem<SelectionModeWindow>();
        viewport = &Tools.systems.AddSystem<Viewport>();

        // Setup Object ID pass
        Tools.Renderer.OnEndCamera += [](render::RenderContext& ctx)
        {
            extern class chisel::Chisel Chisel;
            
            Tools.BeginSelectionPass(ctx);
            Chisel.Renderer->DrawSelectionPass();
        };

        Tools.Loop();
        Tools.Shutdown();
    }
    
    bool Chisel::LoadVMF(std::string_view path)
    {
        auto text = fs::readFileText(path);
        if (!text)
            return false;
        
        auto kv = KeyValues::Parse(*text);
        if (!kv)
            return false;

        VMF::VMF vmf(*kv);
        std::vector<CSG::Side> sides;
        for (const auto& solid : vmf.world.solids)
        {
            sides.clear();
            for (uint64_t i = 0; const auto& side : solid.sides)
                sides.emplace_back(CSG::Side{ { .userdata = i++ }, CSG::Plane{ side.plane.point_trio[0], side.plane.point_trio[1], side.plane.point_trio[2] } });

            Solid& brush = map.AddBrush();
            brush.GetBrush().SetSides(&sides.front(), &sides.back() + 1);
        }
        for (auto& entity : vmf.entities)
        {
            if (!entity.solids.empty())
                continue;

            PointEntity point_entity;
            point_entity.classname = entity.classname;
            point_entity.targetname = entity.targetname;
            point_entity.origin = entity.origin;
            Console.Log("origin: {}", point_entity.origin);
            point_entity.kv = std::move(entity.kv);
            point_entity.connections = std::move(entity.connections);
            map.entities.push_back(point_entity);
        }

        return true;
    }
}

namespace chisel::commands
{
    static ConCommand quit("quit", "Quit the application", []() {
        Tools.Shutdown();
        exit(0);
    });
    
    static ConCommand open_vmf("open_vmf", "Load a VMF from a file path.", [](ConCmd& cmd)
    {
        if (cmd.argc != 1)
            return Console.Error("Usage: open_vmf <path>");

        if (!Chisel.LoadVMF(cmd.argv[0]))
            Console.Error("Failed to load VMF '{}'", cmd.argv[0]);
    });
}

int main(int argc, char* argv[])
{
    using namespace chisel;
    Chisel.Run();
}
