
#include "chisel/Hammer.h"
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

namespace engine::hammer
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

        // Add hammer systems...
        Renderer = &Tools.systems.AddSystem<hammer::MapRender>();
        Tools.systems.AddSystem<hammer::Keybinds>();
        Tools.systems.AddSystem<hammer::Layout>();
        Tools.systems.AddSystem<hammer::SelectionModeWindow>();
        viewport = &Tools.systems.AddSystem<Viewport>();

        // Setup Object ID pass
        Tools.Renderer.OnEndCamera += [](render::RenderContext& ctx)
        {
            Tools.BeginSelectionPass(ctx);

            class hammer::Hammer& Hammer = hammer::Hammer;
            Hammer.Renderer->DrawSolidsWith([&](MapEntity& ent, Solid& solid)
            {
                Tools.PreDrawSelection(ctx.r, hammer::Hammer.GetSelectionID(ent, solid));
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
    using namespace engine::hammer;
    Hammer.Run();
}
