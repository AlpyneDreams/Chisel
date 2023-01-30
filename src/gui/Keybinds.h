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
            if (ImGui::GetIO().KeyCtrl)
            {
                if (Keyboard.GetKeyDown(Key::O)) {
                    Layout::OpenFilePicker();
                }
            }
            
            Map& map = Chisel.map;
            
            if (Selection.Any())
            {
                if (Brush* selected = map.GetBrush(Selection.Active()))
                {
                    // Delete: Remove selected brushes
                    if (Keyboard.GetKeyUp(Key::Delete))
                    {
                        Selection.Clear();
                        map.RemoveBrush(*selected);
                    }

                }
            }
        }
    };
}
