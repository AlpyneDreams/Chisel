
#include "Editor.h"

#include "engine/Engine.h"
#include "imgui/ConsoleWindow.h"
#include "editor/Keybinds.h"
#include "editor/gui/Layout.h"
#include "editor/gui/Outline.h"
#include "editor/gui/Inspector.h"
#include "editor/gui/SceneView.h"
#include "editor/gui/AssetBrowser.h"
#include "editor/Gizmos.h"

#include "entity/components/Transform.h"
#include "entity/components/Camera.h"
#include "render/TextureFormat.h"

#include "editor/Tools.h"

#include <bit>


namespace engine::editor
{
    void Editor::Run()
    {
        // Add editor systems...
        Engine.systems.AddSystem<editor::Keybinds>();
        Engine.systems.AddSystem<editor::Layout>();
        outline       = &Engine.systems.AddSystem<editor::Outline>();
        inspector     = &Engine.systems.AddSystem<editor::Inspector>();
        sceneView     = &Engine.systems.AddSystem<editor::SceneView>();
        assetBrowser  = &Engine.systems.AddSystem<editor::AssetBrowser>();

        Tools.Init();
        Tools.Loop();
        Tools.Shutdown();
    }
}