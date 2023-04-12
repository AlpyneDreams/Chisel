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

        void BeginRow(FGD::Var& var, Entity* ent);
        void VarLabel(const char* name);
        void VarLabel(FGD::Var& var);

        void DrawProperties(FGD::Class* cls, Entity* ent, bool root = true);
        bool ValueInput(FGD::Var& var, Entity* ent, bool raw = false);
        bool ValueInput(FGD::Var& var, std::string* value);
        bool ValueInput(const char* name, FGD::Var& var, std::string* value);

        bool StartTable() {
            return ImGui::BeginTable("properties", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_PadOuterX);
        }

        static constexpr uint32 ModifiedColor = 0xff332e1f;
    };
}
