#pragma once

#include "common/System.h"
#include "gui/Layout.h"
#include "input/Keyboard.h"
#include "chisel/Chisel.h"

#include <imgui.h>

namespace chisel
{
    struct Keybinds : System
    {
        void Update() override
        {
            if (Keyboard.ctrl)
            {
                if (Keyboard.GetKeyDown(Key::O)) {
                    Layout::OpenFilePicker();
                }
            }
            
            Map& map = Chisel.map;
            if (Keyboard.GetKeyUp(Key::Delete)) {
                Selection.Delete();
            }
        }
    };
}
