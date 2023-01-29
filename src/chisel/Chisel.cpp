
#include "chisel/Chisel.h"
#include "chisel/MapRender.h"
#include "common/String.h"
#include "chisel/VMF/KeyValues.h"

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
    void Chisel::Open(const char* path)
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

        //Open("/home/alpyne/Desktop/test.vmf");
        Tools.Loop();
        Tools.Shutdown();
    }
}

namespace chisel::commands
{
    inline ConCommand quit("quit", "Quit the application", []() {
        Tools.Shutdown();
        exit(0);
    }); 
}

int main(int argc, char* argv[])
{
    using namespace chisel;
    Chisel.Run();
}
