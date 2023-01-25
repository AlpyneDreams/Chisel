#pragma once

#include "core/System.h"
#include "chisel/gui/Layout.h"
#include "input/Keyboard.h"
#include <imgui.h>

namespace chisel
{
    struct Keybinds : System
    {
        void Update() override
        {
            if (ImGui::GetIO().KeyCtrl)
            {
                if (Keyboard.GetKeyDown(Key::O)) {
                    Layout::OpenFilePicker();
                }
            }
        }
    };
}
