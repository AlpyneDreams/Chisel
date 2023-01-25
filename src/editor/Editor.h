#pragma once

#include "entity/Entity.h"
#include "imgui/Window.h"
#include "render/Render.h"
#include "editor/Selection.h"

namespace engine::editor
{
    /**
     * Editor is the main level editor.
     */
    inline class Editor
    {
    public:
    // GUI //
        // TODO: Support multiple instances of each
        GUI::Window* outline;
        GUI::Window* inspector;
        GUI::Window* sceneView;
        GUI::Window* assetBrowser;

        Entity AddEntity()
        {
            Entity ent = World.CreateEntity();
            Selection.Select(ent);
            return ent;
        }

        Entity Duplicate(Entity& ent)
        {
            if (!ent)
                return EntityNull;
            
            // Increment number after entity name
            // "Entity" -> "Entity (1)" -> "Entity (2)" -> ...
            std::string name = ent.GetName();
            if (name.ends_with(")") && name.size() >= 3) {
                size_t pos = name.find_last_of('(');
                if (pos != std::string::npos)
                {
                    std::string number = name.substr(pos+1, name.size()-2);
                    try {
                        int num = std::stoi(number);
                        name = name.substr(0, pos) + "(" + std::to_string(num+1) + ")";
                    } catch (...) {
                        name += " (1)";
                    }
                }
                else
                    name += " (1)";
            } else
                name += " (1)";

            Entity clone = ent.Clone();
            clone.SetName(name);
            Selection.Select(clone);
            return clone;
        }
    
    // Editor Engine Loop //

        void Run();

    } Editor;
}