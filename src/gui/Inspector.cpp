#include "Inspector.h"

#include "imgui.h"
#include "chisel/Chisel.h"
#include "chisel/FGD/FGD.h"
#include "chisel/map/Map.h"
#include "chisel/Selection.h"
#include "gui/IconsMaterialCommunity.h"

#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <unordered_set>

namespace chisel
{
    Inspector::Inspector() : GUI::Window(ICON_MC_INFORMATION, "Inspector", 512, 512, true, ImGuiWindowFlags_MenuBar)
    {
    }

    void Inspector::Draw()
    {
        if (ImGui::BeginMenuBar())
        {
            if (debug)
                ImGui::TextUnformatted("Debug");
            
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 60);

            // Draw lock toggle button
            if (GUI::MenuBarButton(locked ? ICON_MC_LOCK : ICON_MC_LOCK_OPEN_OUTLINE)) {
                locked = !locked;
            }
            
            // Draw options menu
            ImGui::SameLine();
            if (ImGui::BeginMenu(ICON_MC_DOTS_VERTICAL))
            {
                ImGui::MenuItem("Debug", "", &debug);
                ImGui::MenuItem("Lock", "", &locked);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // If not locked, inspect current selection
        if (!locked)
        {
            if (Selection.Count() != 1) 
                return;
            
            // TODO: Better interface for this
            target = dynamic_cast<Entity*>(Selection[0]);
            if (!target) {
                locked = false;
                return;
            }
            
            DrawEntityInspector(target);
        }
    }

    static inline void DrawProperties(FGD::Class* cls, Entity* ent, bool root = true);

    void Inspector::DrawEntityInspector(Entity* ent)
    {
        if (ImGui::BeginCombo("Class", ent->classname.c_str()))
        {
            for (auto& [name, cls] : Chisel.fgd->classes)
            {
                bool selected = ent->classname == name;
                if (ImGui::Selectable(name.c_str(), selected)) {
                    ent->classname = name;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        FGD::Class cls = Chisel.fgd->classes[ent->classname];

        DrawProperties(&cls, ent);
    }
    
    static inline bool ValueInput(const char* name, FGD::Var& var, std::string* value)
    {
        using enum FGD::VarType;
        auto type = var.type;
        float v[3] = {0, 0, 0};
        int i = 0;
        bool b = *value == "1";
        switch (type)
        {
            default:        return ImGui::TextUnformatted(value->c_str()), false;
            case Integer:   return ImGui::InputInt(name, &i);
            case Float:     return ImGui::InputFloat(name, &v[0]);
            case Boolean:   return ImGui::Checkbox(name, &b);
            case TargetSrc:
            case TargetDest:
            case TargetNameOrClass: // TODO: Entity pickers
            case String:    return ImGui::InputText(name, value);
            case Choices:
            {
                std::string str = *value;
                if (var.choices.contains(str))
                    str = var.choices[str];

                if (!ImGui::BeginCombo(name, str.c_str()))
                    return false;

                bool modified = false;

                for (auto& [key, name] : var.choices)
                {
                    bool selected = *value == key;
                    if (ImGui::Selectable(name.c_str(), selected)) {
                        modified = true;
                        *value = key;
                    }
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
                return modified;
            }

            case Color255:
            case Color1:    return ImGui::ColorEdit4(name, v);

            case Angle:
            case Vector:
            case Origin:    return ImGui::InputFloat3(name, v);

            case ScriptList:    return ImGui::Button(ICON_MC_SCRIPT " Scripts");
            case Script:        return ImGui::Button(ICON_MC_SCRIPT " Script");
        }
    }

    static inline bool ValueInput(FGD::Var& var, Entity* ent)
    {
        std::string str = "";

        // Get value or default value
        if (ent->kv.contains(var.name))
            str = ent->kv[var.name];
        else if (!var.defaultValue.empty())
            str = var.defaultValue;

        if (var.readOnly)
            ImGui::BeginDisabled();

        bool modified = ValueInput((std::string("##") + var.name).c_str(), var, &str);

        if (var.readOnly)
            ImGui::EndDisabled();
        else if (modified)
            {} // TODO: Update var

        return modified;
    }

    static inline void DrawProperties(FGD::Class* cls, Entity* ent, bool root)
    {
        static std::unordered_set<std::string> visited;
        if (root)
            visited.clear();

        visited.insert(cls->name);

        if (!root)
            for (FGD::Class* base : cls->bases)
                if (!visited.contains(base->name))
                    DrawProperties(base, ent, false);

        if (cls->variables.empty())
            return;

        if (!ImGui::CollapsingHeader(cls->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            return;

        if (!ImGui::BeginTable("properties", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
            return;

        //ImGui::TableSetupColumn("Property Name");
        //ImGui::TableSetupColumn("Value");
        //ImGui::TableHeadersRow();

        for (int varNum = 0, varCount = cls->variables.size(); auto& var : cls->variables)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn();

            // Some FGDs have separators that are purely cosmetic
            if (var.readOnly && var.displayName.find_first_not_of("-") == std::string::npos)
            {
                // Skip separators at the beginning and end
                if (varNum > 0 && varNum < varCount - 1) {
                    ImGui::Separator();
                    ImGui::TableNextColumn();
                    ImGui::Separator();
                }
                varNum++;
                continue;
            }

            
            ImGui::TextUnformatted(var.displayName.c_str());
            ImGui::TableNextColumn();

            //ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.7);
            //GUI::ItemLabel(var.displayName.c_str());
            ImGui::SetNextItemWidth(-FLT_MIN);
            
            //ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ValueInput(var, ent);

            varNum++;
        }

        ImGui::EndTable();

        if (root)
            for (FGD::Class* base : cls->bases)
                if (!visited.contains(base->name))
                    DrawProperties(base, ent, false);
    }
}
