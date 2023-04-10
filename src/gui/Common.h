#pragma once

#include <string_view>

#include "render/Render.h"
#include "Window.h"

struct ImFont;

namespace chisel
{
    namespace GUI
    {
        void Setup();
        void Update();
        void Render();

        inline ImFont* FontMonospace = nullptr;

        // Aligned (left or right) item label (use before input controls)
        void ItemLabel(std::string_view title, bool right = false);

        bool MenuBarButton(const char* title, const char* tooltip = nullptr);
        bool MenuBarToggle(const char* title, bool* v, const char* tooltip = nullptr);

        void WindowToggleButton(Window* window, float width = 64.0f, const char* tooltip = nullptr);

        bool Thumbnail(const char* name, Texture* icon = nullptr, bool selected = false);
    }
}