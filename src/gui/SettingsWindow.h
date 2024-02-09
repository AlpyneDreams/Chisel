#pragma once
#include "gui/Window.h"

namespace chisel
{
    struct SettingsWindow final : public GUI::Window
    {
        SettingsWindow();

        virtual void Draw() override;
    };
}
