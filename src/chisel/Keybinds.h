#pragma once

#include "chisel/Editor.h"
#include "core/System.h"
#include "input/Keyboard.h"
#include <imgui.h>

namespace chisel
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
