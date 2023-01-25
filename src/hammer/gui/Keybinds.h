#pragma once

#include "engine/System.h"
#include "hammer/gui/Layout.h"
#include "input/Keyboard.h"
#include <imgui.h>

namespace engine::hammer
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
