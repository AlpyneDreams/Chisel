#include "Inspector.h"

#include "imgui.h"
#include "common/Hash.h"
#include "chisel/Chisel.h"
#include "chisel/FGD/FGD.h"
#include "chisel/map/Map.h"
#include "chisel/Selection.h"
#include "assets/Assets.h"
#include "gui/IconsMaterialCommunity.h"

#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <unordered_set>

namespace chisel
{
    // These classes won't get a section, but their properties
    // will be shown unless also hidden by HiddenVariables
    const std::unordered_set<Hash> HiddenClasses =
    {
        "EnableDisable"_hash
    };

    const std::unordered_set<Hash> HiddenVariables =
    {
        // EnableDisable
        "startdisabled"_hash
    };

    Inspector::Inspector() : GUI::Window(ICON_MC_INFORMATION, "Inspector", 512, 512, true, ImGuiWindowFlags_MenuBar)
    {
        defaultIcon = Assets.Load<Texture>("textures/ui/entity.png");
        defaultIconBrush = Assets.Load<Texture>("textures/ui/cube.png");
    }

    void Inspector::Draw()
    {
        if (ImGui::BeginMenuBar())
        {
            if (debug)
                ImGui::TextUnformatted("Debug");
            
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);

            // Draw lock toggle button
            GUI::MenuBarToggle(locked ? ICON_MC_LOCK : ICON_MC_LOCK_OPEN_OUTLINE, &locked, "Lock Selection");
            
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

    void Inspector::DrawEntityInspector(Entity* ent)
    {
        FGD::Class cls = Chisel.fgd->classes[ent->classname];

        constexpr float iconSize = 64;
        constexpr float iconPadding = 8;

        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        ImVec2 endPos = ImVec2(screenPos.x + iconSize, screenPos.y + iconSize);

        // Draw entity icon
        Texture* tex = cls.texture;
        if (!tex)
            tex = cls.type == FGD::SolidClass ? defaultIconBrush : defaultIcon;
        if (tex)
            ImGui::GetWindowDrawList()->AddImage(
                (cls.texture ? cls.texture : defaultIcon)->srvLinear.ptr(),
                screenPos, endPos,
                ImVec2(0, 0), ImVec2(1, 1)
            );

        ImVec2 cursorPos = ImGui::GetCursorPos();
        ImGui::SetCursorPos({cursorPos.x + iconSize + iconPadding, cursorPos.y});

        // Draw enable/disable checkbox
        if (cls.EnableDisable)
        {
            FGD::Var& startdisabled = cls.EnableDisable->variables.front();
            if (startdisabled.hash == "startdisabled"_hash)
            {
                std::string invValue = ent->kv.contains("startdisabled") ? ent->kv["startdisabled"] : "0";
                std::string value = invValue == "1" ? "0" : "1";
                if (ValueInput(startdisabled, &value))
                    ent->kv["startdisabled"] = value == "1" ? "0" : "1";
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Start Enabled");
                ImGui::SameLine();
            }
        }

        // Draw classname picker
        if (ImGui::BeginCombo("##Class", ent->classname.c_str()))
        {
            for (auto& [name, cls] : Chisel.fgd->classes)
            {
                if (cls.type == FGD::BaseClass)
                    continue;

                bool selected = ent->classname == name;
                if (ImGui::Selectable(name.c_str(), selected)) {
                    ent->classname = name;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SetCursorPos({cursorPos.x, cursorPos.y + iconSize + iconPadding});

        DrawProperties(&cls, ent);
    }

    // TODO: Variants
    inline bool Inspector::ValueInput(const char* name, FGD::Var& var, std::string* value)
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
            case Boolean:
            {
                if (ImGui::Checkbox(name, &b)) {
                    *value = b ? "1" : "0";
                    return true;
                }
                return false;
            }
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
            case Flags:
            {
                if (ImGui::Button(ICON_MC_FLAG " Flags"))
                    ImGui::OpenPopup("flags");

                bool modified = false;

                if (ImGui::BeginPopup("flags"))
                {
                    for (auto& [key, name] : var.choices)
                    {
                        ImGui::Checkbox(name.c_str(), &b);
                    }
                    ImGui::EndPopup();
                }
                return modified;
            }

            case Color255:
            case Color1:    return ImGui::ColorEdit4(name, v);

            case Angle:
            case Vector:
            case Origin:    return ImGui::DragFloat3(name, v);

            case ScriptList:    return ImGui::Button(ICON_MC_SCRIPT " Scripts");
            case Script:        return ImGui::Button(ICON_MC_SCRIPT " Script");
        }
    }

    inline bool Inspector::ValueInput(FGD::Var& var, std::string* value)
    {
        return ValueInput((std::string("##") + var.name).c_str(), var, value);
    }

    inline bool Inspector::ValueInput(FGD::Var& var, Entity* ent)
    {
        std::string str = "";

        // Get value or default value
        if (ent->kv.contains(var.name))
            str = ent->kv[var.name];
        else if (!var.defaultValue.empty())
            str = var.defaultValue;

        if (var.readOnly)
            ImGui::BeginDisabled();

        bool modified = ValueInput(var, &str);

        if (var.readOnly)
            ImGui::EndDisabled();
        else if (modified)
            {} // TODO: Update var

        return modified;
    }

    inline bool Inspector::RawInput(FGD::Var& var, Entity* ent)
    {
        std::string str = "";

        // Get value or default value
        if (ent->kv.contains(var.name))
            str = ent->kv[var.name];
        else if (!var.defaultValue.empty())
            str = var.defaultValue;

        if (var.readOnly)
            ImGui::BeginDisabled();

        bool modified = ImGui::InputText((std::string("##") + var.name).c_str(), &str);

        if (var.readOnly)
            ImGui::EndDisabled();
        else if (modified && str != var.defaultValue)
            ent->kv[var.name] = str;

        return modified;
    }

    void Inspector::DrawProperties(FGD::Class* cls, Entity* ent, bool root)
    {
        static std::unordered_set<Hash> visited;
        if (root)
            visited.clear();

        visited.insert(cls->hash);

        // Draw nested base classes first
        if (!root)
            for (FGD::Class* base : cls->bases)
                if (!visited.contains(base->hash))
                    DrawProperties(base, ent, false);

        // Draw (sub)class header
        bool showHeader = debug || (
            !HiddenClasses.contains(cls->hash) && !cls->variables.empty()
        );
        if (showHeader)
        {
            if (!ImGui::CollapsingHeader(cls->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                goto BaseClasses;
        }

        if (!ImGui::BeginTable("properties", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_PadOuterX))
            return;

        //ImGui::TableSetupColumn("Property Name");
        //ImGui::TableSetupColumn("Value");
        //ImGui::TableHeadersRow();
        
        if (root)
        {
            // Draw unrecognized properties
            for (auto& [key, value] : ent->kv)
            {
                auto hash = HashString(key);
                if (!cls->HasVar(hash))
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, 0xff331f33);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(key.c_str());
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputText((std::string("##") + key).c_str(), &value);
                }
            }

            // Draw special properties
            if (cls->type != FGD::SolidClass)
            {
                if (PointEntity* point = dynamic_cast<PointEntity*>(ent))
                {
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextUnformatted(debug ? "origin" : "Position");
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::DragFloat3("##position", &point->origin.x);
                }
            }
        }

        for (int varNum = 0, varCount = cls->variables.size(); auto& var : cls->variables)
        {
            if (!debug && HiddenVariables.contains(var.hash))
                continue;

            ImGui::TableNextRow();

            if (ent->kv.contains(var.name) && ent->kv[var.name] != var.defaultValue)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, 0xff332e1f);
            }
            
            ImGui::TableNextColumn();

            if (debug)
            {
                ImGui::TextUnformatted(var.name.c_str());
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                RawInput(var, ent);
                continue;
            }

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

            const char* name = var.displayName.c_str();
            if (var.type == FGD::Flags && var.displayName == "spawnflags")
                name = "Flags";

            ImGui::TextUnformatted(name);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ValueInput(var, ent);

            varNum++;
        }

        ImGui::EndTable();

    BaseClasses:

        if (root)
            for (FGD::Class* base : cls->bases)
                if (!visited.contains(base->hash))
                    DrawProperties(base, ent, false);
    }
}

