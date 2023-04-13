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
    // will still be shown unless also hidden or hoisted
    const std::unordered_set<Hash> HiddenClasses =
    {
        "EnableDisable"_hash
    };

    const std::vector<Hash> HoistedVariables =
    {
        "targetname"_hash,
        "origin"_hash,          // All PointClass entities
        "spawnflags"_hash,
        "startdisabled"_hash    // EnableDisable
    };

    const std::unordered_set<Hash> HoistedVariableSet = {HoistedVariables.begin(), HoistedVariables.end()};

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
        const FGD::Class& cls = Chisel.fgd->classes[ent->classname];

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

        // Draw classname picker
        ClassnamePicker(&ent->classname, cls.type == FGD::SolidClass);

        ImGui::SetCursorPos({cursorPos.x, cursorPos.y + iconSize + iconPadding});
        ImGui::Separator();

        DrawProperties(&cls, ent);
    }

    void Inspector::ClassnamePicker(std::string* classname, bool solids, const char* label)
    {
        if (ImGui::BeginCombo(label, classname->c_str()))
        {
            for (auto& [name, cls] : Chisel.fgd->classes)
            {
                if (cls.type == FGD::BaseClass)
                    continue;

                if (solids) {
                    if (cls.type != FGD::SolidClass)
                        continue;
                } else {
                    if (cls.type == FGD::SolidClass)
                        continue;
                }

                bool selected = *classname == name;
                if (ImGui::Selectable(name.c_str(), selected)) {
                    *classname = name;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    // TODO: Variants
    inline bool Inspector::ValueInput(const char* name, const FGD::Var& var, kv::KeyValuesVariant& kv)
    {
        using enum FGD::VarType;
        bool dummyBool = false;
        switch (var.type)
        {
            default:
            {
                std::string_view view = (std::string_view)kv;
                ImGui::TextUnformatted(view.data(), view.data() + view.length());
                return false;
            }
            case Integer:
            {
                kv.EnsureType(kv::Types::Int);
                return ImGui::InputInt(name, kv.GetPtr<int32_t>(kv::Types::Int));
            }
            case Float:
            {
                kv.EnsureType(kv::Types::Float);
                return ImGui::InputDouble(name, kv.GetPtr<double>(kv::Types::Float));
            }

            case Boolean:
            {
                if (kv.GetType() == kv::Types::Int)
                {
                    bool* value = (bool*)kv.GetPtr<int64_t>(kv::Types::Int);
                    if (ImGui::Checkbox(name, value))
                    {
                        *value = *value ? "1" : "0";
                        return true;
                    }
                    return false;
                }
                else
                {
                    kv.EnsureType(kv::Types::Uint64);
                    bool* value = (bool*)kv.GetPtr<uint64_t>(kv::Types::Uint64);
                    if (ImGui::Checkbox(name, value))
                    {
                        *value = *value ? "1" : "0";
                        return true;
                    }
                    return false;
                }
            }
            case StudioModel:
            case Sound:
            case TargetSrc:
            case TargetDest:
            case TargetNameOrClass: // TODO: Entity pickers
            case String:
            {
                kv.EnsureType(kv::Types::String);
                return ImGui::InputText(name, kv.GetPtr<std::string>(kv::KeyValuesType::String));
            }
            case Choices:
            {
                std::string str = std::string((std::string_view)kv);
                if (var.choices.contains(str))
                    str = var.choices.at(str);

                if (!ImGui::BeginCombo(name, str.c_str()))
                    return false;

                bool modified = false;

                for (auto& [key, name] : var.choices)
                {
                    bool selected = kv == key;
                    if (ImGui::Selectable(name.c_str(), selected))
                    {
                        modified = true;
                        kv = std::move(kv::KeyValuesVariant::Parse(key));
                    }
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
                return modified;
            }
#if 0
            case Flags:
            {
                if (ImGui::Button(ICON_MC_FLAG " Flags"))
                    ImGui::OpenPopup("flags");

                bool modified = false;

                if (ImGui::BeginPopup("flags"))
                {
                    for (auto& [key, name] : var.choices)
                    {
                        ImGui::Checkbox(name.c_str(), &dummyBool);
                    }
                    ImGui::EndPopup();
                }
                return modified;
            }
#endif

            case Color255:
            case Color1:
            {
                kv.EnsureType(kv::Types::Vector4);
                return ImGui::ColorEdit4(name, (float*)kv.GetPtr<vec4>(kv::Types::Vector4), ImGuiColorEditFlags_Float);
            }

            case Angle:
            case Vector:
            case Origin:
            {
                kv.EnsureType(kv::Types::Vector3);
                return ImGui::DragFloat3(name, (float*)kv.GetPtr<vec3>(kv::Types::Vector3));
            }

            case ScriptList:    return ImGui::Button(ICON_MC_SCRIPT " Scripts");
            case Script:        return ImGui::Button(ICON_MC_SCRIPT " Script");
        }
    }

    inline bool Inspector::ValueInput(const FGD::Var& var, kv::KeyValuesVariant &value)
    {
        return ValueInput((std::string("##") + var.name).c_str(), var, value);
    }

    inline bool Inspector::ValueInput(const FGD::Var& var, Entity* ent, bool raw)
    {
        kv::KeyValuesVariant *kv = nullptr;
        bool defaultVal = false;

        // Get value or default value
        if (ent->kv.Contains(var.name))
            kv = &ent->kv[var.name];
        else
        {
            kv = &ent->kv.CreateChild(var.name, var.defaultValue.c_str());
            defaultVal = true;
        }

        if (!kv)
            return false;

        if (!defaultVal)
            defaultVal = kv->Get<std::string_view>() == var.defaultValue;

        if (var.readOnly)
            ImGui::BeginDisabled();

        float cursorX, width = -FLT_MIN;

        // Make space for reset button
        if (!defaultVal)
        {
            cursorX = ImGui::GetCursorPosX();
            width = ImGui::GetContentRegionAvail().x - 26.;
        }
        ImGui::SetNextItemWidth(width);

#if 0
        // TODO JEFF WHAT WAS THIS FOR I DONT KNOW
        // FIX IT
        bool modified = raw
            ? ImGui::InputText((std::string("##") + var.name).c_str(), )
            : ValueInput(var, &str);
#endif
        bool modified = ValueInput(var, *kv);

        // Reset button
        if (!defaultVal)
        {
            ImGui::SameLine();
            ImGui::SetCursorPosX(cursorX + width + 2.f);
            if (defaultVal)
                ImGui::BeginDisabled();

            if (ImGui::Button(ICON_MC_ARROW_U_LEFT_TOP)) {
                //str = var.defaultValue;
                modified = true;
            }

            if (defaultVal)
                ImGui::EndDisabled();
        }

        if (var.readOnly)
            ImGui::EndDisabled();
        //else if (modified)
            //ent->kv[var.name] = str;

        return modified;
    }

    inline void Inspector::BeginRow(const FGD::Var& var, Entity* ent)
    {
        ImGui::TableNextRow();
        
        if (ent->kv.Contains(var.name) && ent->kv[var.name] != var.defaultValue)
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ModifiedColor);

        ImGui::TableNextColumn();
    }

    inline void Inspector::VarLabel(const char* name)
    {
        ImGui::TextUnformatted(name);

        ImGui::TableNextColumn();
    }

    inline void Inspector::VarLabel(const FGD::Var& var)
    {
        const char* name = var.displayName.c_str();
        if (var.type == FGD::Flags && var.displayName == "spawnflags")
            name = "Flags";

        VarLabel(name);
    }

    void Inspector::DrawProperties(const FGD::Class* cls, Entity* ent, bool root)
    {
        static std::unordered_set<Hash> visited;
        if (root)
            visited.clear();

        visited.insert(cls->hash);

        // Draw nested base classes first
        if (!root)
            for (const FGD::Class* base : cls->bases)
                if (!visited.contains(base->hash))
                    DrawProperties(base, ent, false);

        //ImGui::TableSetupColumn("Property Name");
        //ImGui::TableSetupColumn("Value");
        //ImGui::TableHeadersRow();
        
        if (root)
        {
            if (!StartTable())
                return;
            // Draw unrecognized properties
#if 0
            for (auto& [key, value] : ent->kv)
            {
                auto hash = HashString(key);
                if (!cls->GetVar(hash))
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
#endif

            // Draw hoisted and special properties
            for (Hash hash : HoistedVariables)
            {
                if (const FGD::Var* var = cls->GetVar(hash); var && !debug)
                {
                    BeginRow(*var, ent);
                    VarLabel(*var);
                    ValueInput(*var, ent);
                }
                else if (hash == "origin"_hash && cls->type != FGD::SolidClass)
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

            ImGui::EndTable();
        }

        // Draw (sub)class header
        bool showHeader = debug || (
            !HiddenClasses.contains(cls->hash)
            && !cls->variables.empty()
            && !(cls->variables.size() == 1 && HoistedVariableSet.contains(cls->variables.begin()->first))
        );
        if (showHeader)
        {
            if (!ImGui::CollapsingHeader(cls->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                goto BaseClasses;
        }

        if (!StartTable())
            return;

        for (int varNum = 0, varCount = cls->variables.size(); auto& [hash, var] : cls->variables)
        {
            if (!debug && HoistedVariableSet.contains(hash))
                continue;

            BeginRow(var, ent);

            if (debug)
            {
                VarLabel(var.name.c_str());
                ValueInput(var, ent, true);
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

            VarLabel(var);
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

