#pragma once

#include "common/Common.h"
#include "common/System.h"

#include "imgui.h"
#include "gui/Common.h"
#include "gui/Toolbar.h"

namespace chisel
{
    enum class SelectMode;

    struct SelectionModeToolbar final : Toolbar
    {
        SelectionModeToolbar();

        void DrawToolbar() override;
        void Option(const char* name, SelectMode mode);
    };

    struct Layout : System
    {
        Space space = Space::World;

        void Start() override;
        void Update() override;

        static void OpenFilePicker();
    };
}
