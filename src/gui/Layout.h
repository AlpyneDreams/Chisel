#pragma once

#include "common/Common.h"
#include "common/System.h"

#include "imgui.h"
#include "gui/Common.h"
#include "gui/Toolbar.h"

namespace chisel
{
    enum class SelectMode;
    struct Tool;

    struct SelectionModeToolbar final : Toolbar
    {
        SelectionModeToolbar();

        void DrawToolbar() override;
        void Option(const char* name, SelectMode mode);
    };

    struct EditingToolbar final : Toolbar
    {
        EditingToolbar();

        void DrawToolbar() override;

        // Should match default values of view_grid_size
        bool uniformGridSize = true;
        int3 gridPower = int3(6); // 64
    };

    struct MainToolbar final : Toolbar
    {
        MainToolbar();

        void DrawToolbar() override;
        void Option(const char* icon, const char* name, Tool* mode);
    };

    struct Layout : System
    {
        Space space = Space::World;

        void Update() override;

        static void OpenFilePicker();
        static void SaveFilePicker();
    };
}
