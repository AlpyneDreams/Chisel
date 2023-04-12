#pragma once

#include "gui/Common.h"
#include "gui/Window.h"
#include "chisel/FGD/FGD.h"

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

        void DrawProperties(FGD::Class* cls, Entity* ent, bool root = true);
        bool ValueInput(FGD::Var& var, Entity* ent);
        bool RawInput(FGD::Var& var, Entity* ent);
        bool ValueInput(FGD::Var& var, std::string* value);
        bool ValueInput(const char* name, FGD::Var& var, std::string* value);
    };
}
