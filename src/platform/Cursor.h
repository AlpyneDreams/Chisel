#pragma once

#include "common/Common.h"

#include <imgui.h>

namespace engine
{
    /** Manages cursor appearance and behavior.
     * To get mouse input, use engine::Mouse in input/Input.h
     */
    inline struct Cursor
    {
        enum Mode {
            Normal, Confined, Locked
        };

        void SetMode(Mode mode);
        Mode GetMode() const;

        void SetVisible(bool visible) {
            ImGui::SetMouseCursor(visible ? ImGuiMouseCursor_Arrow : ImGuiMouseCursor_None);
        }
        bool GetVisible() const {
            return ImGui::GetMouseCursor() != ImGuiMouseCursor_None;
        }
    } Cursor;
}