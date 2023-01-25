
#include "hammer/Hammer.h"
#include "hammer/MapRender.h"
#include "common/String.h"
#include "hammer/KeyValues.h"

#include "console/Console.h"
#include "engine/Engine.h"
#include "hammer/VMF.h"
#include "imgui/ConsoleWindow.h"
#include "hammer/gui/Layout.h"
#include "hammer/gui/Viewport.h"
#include "hammer/gui/Keybinds.h"

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
        Renderer = &Engine.systems.AddSystem<hammer::MapRender>();
        Engine.systems.AddSystem<hammer::Keybinds>();
        Engine.systems.AddSystem<hammer::Layout>();
        Engine.systems.AddSystem<hammer::SelectionModeWindow>();
        viewport = &Engine.systems.AddSystem<Viewport>();

        // Setup Object ID pass
        Tools.Renderer.OnEndCamera -= Tools.DrawSelectionPass;
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
