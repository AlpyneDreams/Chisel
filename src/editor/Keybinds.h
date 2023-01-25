#pragma once

#include "editor/Editor.h"
#include "engine/System.h"
#include "input/Keyboard.h"
#include <imgui.h>

namespace engine::editor
{
    struct Keybinds : System
    {
        void Update() override
        {
            if (Entity ent = Selection.Active())
            {
                if (ImGui::GetIO().KeyCtrl)
                {
                    if (Keyboard.GetKeyDown(Key::D)) {
                        Editor.Duplicate(ent);
                    }
                } 
                else if (Keyboard.GetKeyDown(Key::Delete)) {
                    ent.Delete();
                }
            }
        }
    };
}
