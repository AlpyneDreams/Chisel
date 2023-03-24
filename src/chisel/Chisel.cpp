
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
        Tools.systems.AddSystem<MainToolbar>();
        Tools.systems.AddSystem<SelectionModeToolbar>();
        viewport = &Tools.systems.AddSystem<Viewport>();

        // Setup Object ID pass
        Tools.Renderer.OnEndCamera += [](render::RenderContext& ctx)
        {
            extern class chisel::Chisel Chisel;
            
            Tools.BeginSelectionPass(ctx);
            Chisel.Renderer->DrawSelectionPass();
        };

        map.AddCube(glm::translate(mat4x4(1), vec3(0, 0, 64)));

        Tools.Loop();
        Tools.Shutdown();
    }
    
    bool Chisel::LoadVMF(std::string_view path)
    {
        auto text = fs::readTextFile(path);
        if (!text)
            return false;
        
        auto kv = KeyValues::Parse(*text);
        if (!kv)
            return false;

        VMF::VMF vmf(*kv);
        vmf.Import(map);

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
