#pragma once

#include "gui/Common.h"
#include "gui/Window.h"

namespace chisel
{
    struct Entity;

    struct Inspector : public GUI::Window
    {
        Inspector();
    
        bool debug = false;
        bool locked = false;

        Entity* target = nullptr;

        void Draw() override;
        void DrawEntityInspector(Entity* ent);

        Texture* defaultIcon;
        Texture* defaultIconBrush;
    };
}
