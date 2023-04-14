#pragma once

#include "gui/Common.h"
#include "gui/Window.h"
#include "chisel/FGD/FGD.h"
#include "formats/KeyValues.h"

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

        static void ClassnamePicker(std::string* classname, bool solids = false, const char* label = "##Class");

        Texture* defaultIcon;
        Texture* defaultIconBrush;

        void BeginRow(const FGD::Var& var, Entity* ent);
        void VarLabel(const char* name);
        void VarLabel(const FGD::Var& var);

        void DrawProperties(const FGD::Class* cls, Entity* ent, bool root = true);
        bool ValueInput(const FGD::Var& var, Entity* ent, bool raw = false);
        bool ValueInput(const FGD::Var& var, kv::KeyValuesVariant& kv);
        bool RawInput(const FGD::Var& var, kv::KeyValuesVariant& kv);
        bool ValueInput(const char* name, const FGD::Var& var, kv::KeyValuesVariant& kv);

        bool GetKV(const FGD::Var& var, Entity* ent, kv::KeyValuesVariant*& kv);

        bool StartTable() {
            return ImGui::BeginTable("properties", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_PadOuterX);
        }

        static constexpr uint32 ModifiedColor = 0xff332e1f;
    };
}
