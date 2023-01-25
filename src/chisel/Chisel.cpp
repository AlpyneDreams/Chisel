
#include "chisel/Chisel.h"
#include "chisel/MapRender.h"
#include "common/String.h"
#include "chisel/KeyValues.h"

#include "console/Console.h"
#include "chisel/VMF.h"
#include "gui/ConsoleWindow.h"
#include "chisel/gui/Layout.h"
#include "chisel/gui/Viewport.h"
#include "chisel/gui/Keybinds.h"

#include "common/Filesystem.h"
#include "render/Render.h"

#include <cstring>
#include <vector>

namespace chisel
{
    void Hammer::Open(const char* path)
    {
        Console.Log("Open: '{}'", path);
        if (!fs::exists(path)) {
            Console.Error("Error: file '{}' does not exist", path);
            return;
        }

        std::string vmf = fs::readFile(path);

        KeyValues kv = KeyValues::Parse(vmf);

        map = VMF(kv);
    }

    void Hammer::Run()
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
            Tools.BeginSelectionPass(ctx);

            class chisel::Hammer& Hammer = chisel::Hammer;
            Hammer.Renderer->DrawSolidsWith([&](MapEntity& ent, Solid& solid)
            {
                Tools.PreDrawSelection(ctx.r, chisel::Hammer.GetSelectionID(ent, solid));
                ctx.r.DrawMesh(&solid.mesh);
            });
        };

        //Open("/home/alpyne/Desktop/test.vmf");
        Tools.Loop();
        Tools.Shutdown();
    }
}

int main(int argc, char* argv[])
{
    using namespace chisel;
    Hammer.Run();
}