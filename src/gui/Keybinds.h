#pragma once

#include "common/System.h"
#include "gui/Layout.h"
#include "input/Keyboard.h"
#include "chisel/Chisel.h"

#include <imgui.h>

namespace chisel
{
    extern ConVar<vec3> view_grid_size;

    struct Keybinds : System
    {
        void Update() override
        {
            Map& map = Chisel.map;

            if (Keyboard.ctrl)
            {
                if (Keyboard.GetKeyDown(Key::O))
                    Layout::OpenFilePicker();

                else if (Keyboard.GetKeyDown(Key::Space))
                    Chisel.mainAssetPicker->ToggleOrFocus();

                else if (Keyboard.GetKeyDown(Key::Z))
                    map.Actions().Undo();

                else if (Keyboard.GetKeyDown(Key::Y))
                    map.Actions().Redo();
            }

            if (Keyboard.GetKeyUp(Key::LeftBracket) || Keyboard.GetKeyUp(Key::RightBracket))
            {
                const bool up = Keyboard.GetKeyUp(Key::RightBracket);
                if (up)
                    view_grid_size.value *= 2.0f;
                else
                    view_grid_size.value /= 2.0f;

                view_grid_size = glm::clamp(view_grid_size.value, glm::vec3(1.0f / 32.0f), glm::vec3(16384.0f));
            }

            if (Keyboard.GetKeyUp(Key::Delete)) {
                Selection.Delete();
            }
        }
    };
}
