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
        defaultIcons[0] = Assets.Load<Texture>("textures/ui/entity.png");
        defaultIcons[1] = Assets.Load<Texture>("textures/ui/entity2.png");
        defaultIcons[2] = Assets.Load<Texture>("textures/ui/entity3.png");
        defaultIcons[3] = Assets.Load<Texture>("textures/ui/entity4.png");
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
            Entity* target_ent = dynamic_cast<Entity*>(Selection[0]);
            Face* target_face = dynamic_cast<Face*>(Selection[0]);
            if (!target_ent && !target_face) {
                locked = false;
                return;
            }

            if (target_ent)
                DrawEntityInspector(target_ent);
            else if (target_face)
                DrawFaceInspector(target_face);
        }
    }

    void Inspector::DrawEntityInspector(Entity* ent)
    {
        const FGD::Class& cls = Chisel.fgd->classes[ent->classname];

        constexpr float iconSize = 64;
        constexpr float iconPadding = 8;

        bool hasHelp = !cls.description.empty();
        bool hasDefaultIcon = false;

        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        ImVec2 endPos = ImVec2(screenPos.x + iconSize, screenPos.y + iconSize);
        ImVec2 cursorPos = ImGui::GetCursorPos();

        Texture* defaultIcon = defaultIcons[defaultIconIndex].ptr();

        // Draw entity icon
        Texture* tex = cls.texture.ptr();
        if (tex == nullptr) {
            tex = cls.type == FGD::SolidClass ? defaultIconBrush.ptr() : defaultIcon;
            hasDefaultIcon = true;
        }
        if (tex) {
            ImGui::GetWindowDrawList()->AddImage(
                (cls.texture.ptr() ? cls.texture.ptr() : defaultIcon)->srvLinear.ptr(),
                screenPos, endPos,
                ImVec2(0, 0), ImVec2(1, 1)
            );
        }

        // Cycle light bulb states :v
        if (ImGui::InvisibleButton("Icon", {iconSize, iconSize}) && hasDefaultIcon)
            defaultIconIndex = (defaultIconIndex + 1) % 4;

        // If icon is hovered, show help text
        if (ImGui::IsItemHovered())
        {
            if (hasHelp)
                GUI::HelpTooltip(cls.description.c_str(), cls.name.c_str());

            if (hasDefaultIcon)
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        ImGui::SetCursorPos({cursorPos.x + iconSize + iconPadding, cursorPos.y});

        // Draw classname picker
        ClassnamePicker(&ent->classname, cls.type == FGD::SolidClass);

        // Draw help icon
        ImGui::BeginDisabled(!hasHelp);
        ImGui::SameLine();
        ImGui::Selectable(ICON_MC_HELP_CIRCLE, false, 0, ImGui::CalcTextSize(ICON_MC_HELP_CIRCLE));
        if (hasHelp && ImGui::IsItemHovered())
            GUI::HelpTooltip(cls.description.c_str(), cls.name.c_str());
        ImGui::EndDisabled();

        ImGui::SetCursorPos({cursorPos.x, cursorPos.y + iconSize + iconPadding});
        ImGui::Separator();

        // Inspect the entity!
        DrawProperties(&cls, ent);
    }

    void Inspector::DrawFaceInspector(Face *face)
    {
        Side *side = face->side;

        constexpr float iconSize = 64;
        constexpr float iconPadding = 8;

        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        ImVec2 endPos = ImVec2(screenPos.x + iconSize, screenPos.y + iconSize);
        ImVec2 cursorPos = ImGui::GetCursorPos();

        auto* srv = side->material != nullptr && side->material->baseTexture != nullptr ? side->material->baseTexture->srvLinear.ptr() : nullptr;
        if (srv) {
            ImGui::GetWindowDrawList()->AddImage(
                srv,
                screenPos, endPos,
                ImVec2(0, 0), ImVec2(1, 1)
            );
        }

        ImGui::SetCursorPos({cursorPos.x + iconSize + iconPadding, cursorPos.y});

        static char inputPath[4096] = {}; // make this LESS SHIT
        if (side->material != nullptr)
        {
            std::string_view path = (std::string_view)side->material->GetPath();
            strncpy(inputPath, path.data(), path.length());
            inputPath[path.length()] = '\0';
        }

        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("Material", inputPath, 4096))
        {
            auto previousMaterial = side->material;
            side->material = Assets.Load<Material>(inputPath);

            if (side->material != previousMaterial)
                face->solid->UpdateMesh();
        }

        ImGui::SetCursorPos({cursorPos.x, cursorPos.y + iconSize + iconPadding});
        ImGui::Separator();

        if (!StartTable())
            return;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        VarLabel("Texture scale", "Change texture scale");
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::DragFloat2("Texture scale", side->scale.data(), 1.0f, 0.0f, 0.0f, "%g", ImGuiSliderFlags_NoRoundToFormat))
            face->solid->UpdateMesh();
        ImGui::TableNextColumn();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        VarLabel("Lightmap scale", "Change lightmap scale");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::DragFloat("Lightmap scale", &side->lightmapScale, 1.0f, 0.0f, 0.0f, "%g", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::TableNextColumn();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        VarLabel("Displacement", "Is this face a displacement?");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::TextUnformatted(side->disp ? "Yes" : "No");
        ImGui::TableNextColumn();

        ImGui::EndTable();
    }

    void Inspector::ClassnamePicker(std::string* classname, bool solids, const char* label)
    {
        ImGui::PushFont(GUI::FontMonospace);
        if (ImGui::BeginCombo("##classname", classname->c_str()))
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
        ImGui::PopFont();
        if (label)
        {
            ImGui::SameLine();
            ImGui::TextUnformatted(label);
        }
    }

    inline bool Inspector::ValueInput(const char* name, const FGD::Var& var, kv::KeyValuesVariant& kv)
    {
        using enum FGD::VarType;
        bool dummyBool = false;
       
        auto type = var.type;
        
        // If a non-string type has a string value (i.e. from VMF
        // or raw edit mode), then make it non-editable
        switch (type)
        {
            case StudioModel:
            case Sound:
            case TargetSrc:
            case TargetDest:
            case TargetNameOrClass:
            case String:
                type = FGD::String;
                break;
            default:
                if (kv.GetType() == kv::Types::String)
                    type = FGD::BadType;
                break;
        }
        
        switch (type)
        {
            default:
            {
                std::string_view view = (std::string_view)kv;
                ImGui::BeginDisabled();
                ImGui::InputText(name, (char*)view.data(), view.length());
                ImGui::EndDisabled();
                return false;
            }
            case Integer:
            {
                kv.EnsureType(kv::Types::Int);
                return ImGui::DragInt(name, kv.GetPtr<int32_t>(kv::Types::Int));
            }
            case Float:
            {
                kv.EnsureType(kv::Types::Float);
                return ImGui::DragScalar(name, ImGuiDataType_Double, kv.GetPtr<double>(kv::Types::Float), 1.f, nullptr, nullptr, "%g", ImGuiSliderFlags_NoRoundToFormat);
            }

            case Boolean:
            {
                kv.EnsureType(kv::Types::Int);
                bool* value = (bool*)kv.GetPtr<int64_t>(kv::Types::Int);
                if (ImGui::Checkbox(name, value))
                {
                    *value = *value ? "1" : "0";
                    return true;
                }
                return false;
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
                for (auto& [key, name] : var.choices)
                {
                    if (key == str)
                    {
                        str = name;
                        break;
                    }
                }

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
            case Color255:
            {
                kv.EnsureType(kv::Types::Vector4);

                vec4 color = kv.Get<vec4>() / vec4(255.f);

                if (ImGui::ColorEdit4(name, &color.r, ImGuiColorEditFlags_HDR)) {
                    kv = color * vec4(255.f);
                    return true;
                }
                return false;
            }
            case Color1:
            {
                kv.EnsureType(kv::Types::Vector4);
                return ImGui::ColorEdit4(name, (float*)kv.GetPtr<vec4>(kv::Types::Vector4),
                    ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            }

            case Angle:
            case Vector:
            case Origin:
            {
                kv.EnsureType(kv::Types::Vector3);
                return ImGui::DragFloat3(name, (float*)kv.GetPtr<vec3>(kv::Types::Vector3), 1.f, 0.f, 0.f, "%g", ImGuiSliderFlags_NoRoundToFormat);
            }

            case ScriptList:    return ImGui::Button(ICON_MC_SCRIPT " Scripts");
            case Script:        return ImGui::Button(ICON_MC_SCRIPT " Script");
        }
    }

    inline bool Inspector::ValueInput(const FGD::Var& var, kv::KeyValuesVariant &value)
    {
        return ValueInput((std::string("##") + var.name).c_str(), var, value);
    }

    inline bool Inspector::RawInput(const FGD::Var& var, kv::KeyValuesVariant &value)
    {
        std::string text = std::string(value.Get<std::string_view>());
        if (ImGui::InputText((std::string("##") + var.name).c_str(), &text)) {
            value = kv::KeyValuesVariant::Parse(text);
            return true;
        }
        return false;
    }

    inline bool Inspector::GetKV(const FGD::Var& var, Entity* ent, kv::KeyValuesVariant*& kv)
    {
        bool defaultVal = false;
        if (ent->kv.Contains(var.name))
            kv = &ent->kv[var.name];
        else
        {
            kv = &ent->kv.CreateChild(var.name, var.defaultValue.c_str());
            defaultVal = true;
        }

        if (!defaultVal)
        {
            if (var.defaultValue.empty())
                defaultVal = kv->IsDefault();
            else
                defaultVal = kv->Get<std::string_view>() == var.defaultValue;
        }

        return defaultVal;
    }

    inline bool Inspector::ValueInput(const FGD::Var& var, Entity* ent, bool raw)
    {
        kv::KeyValuesVariant *kv = nullptr;
        const bool defaultVal = GetKV(var, ent, kv);

        ImGui::PushID(var.name.c_str());
        ImGui::BeginDisabled(var.readOnly);

        float cursorX, width = -FLT_MIN;

        // Make space for reset button
        if (!defaultVal)
        {
            cursorX = ImGui::GetCursorPosX();
            width = ImGui::GetContentRegionAvail().x - 28.;
        }
        ImGui::SetNextItemWidth(width);

        bool modified = raw
            ? RawInput(var, *kv)
            : ValueInput(var, *kv);

        // Reset button
        if (!defaultVal)
        {
            ImGui::SameLine();
            ImGui::SetCursorPosX(cursorX + width + 4.f);
            ImGui::BeginDisabled(defaultVal);

            if (ImGui::Button(ICON_MC_ARROW_U_LEFT_TOP))
            {
                *kv = std::move(kv::KeyValuesVariant::Parse(var.defaultValue));
                modified = true;
            }

            ImGui::EndDisabled();
        }

        if (modified)
            kv->ValueChanged();

        ImGui::EndDisabled();
        ImGui::PopID();

        return modified;
    }

    inline void Inspector::BeginRow(const FGD::Var& var, Entity* ent)
    {
        ImGui::TableNextRow();

        kv::KeyValuesVariant* kv = nullptr;
        bool defaultVal = GetKV(var, ent, kv);
        
        if (!defaultVal)
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ModifiedColor);

        ImGui::TableNextColumn();
    }

    inline void Inspector::VarLabel(const char* name, const char* desc, const char* keyname)
    {
        ImGui::Selectable(debug && keyname ? keyname : name);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            GUI::HelpTooltip(desc, keyname);

        ImGui::TableNextColumn();
    }

    inline bool Inspector::VarTreeNode(const char* name, const char* desc, const char* keyname)
    {
        bool open = ImGui::TreeNodeEx(debug && keyname ? keyname : name,
            ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            GUI::HelpTooltip(desc, keyname);

        ImGui::TableNextColumn();

        return open;
    }

    inline void Inspector::VarLabel(const FGD::Var& var)
    {
        VarLabel(var.displayName.c_str(), var.description.c_str(), var.name.c_str());
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
                if (const FGD::Var* var = cls->GetVar(hash); var && !debug && hash != "spawnflags"_hash)
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
                        VarLabel("Position", "The absolute position of this entity.", "origin");
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::DragFloat3("##position", &point->origin.x, 1.f, 0.f, 0.f, "%g", ImGuiSliderFlags_NoRoundToFormat);
                    }
                }
                else if (var && hash == "spawnflags"_hash)
                {
                    BeginRow(*var, ent);
                    bool open = VarTreeNode(
                        var->displayName == var->name ? "Flags" : var->displayName.c_str(),
                        var->description.empty() ? "Toggle specific features of this entity." : var->description.c_str(),
                        var->name.c_str()
                    );
                    ImGui::TextUnformatted(ICON_MC_FLAG);
                    ImGui::TableNextColumn();
                    if (open)
                    {
                        ImGui::TreePop();
                        for (auto& [key, name] : var->choices)
                        {
                            ImGui::TableNextRow(); ImGui::TableNextColumn();

                            const char* label = name.c_str();
                            if (label[0] == '[' && isdigit(label[1]))
                            {
                                const char* c = &label[1];
                                while (isdigit(*c)) c++;
                                if (*c == ']') {
                                    label = &c[1];
                                    if (*label == ' ')
                                        label++;
                                }
                            }

                            ImGui::Selectable("");
                            bool hover = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
                            ImGui::SameLine();
                            ImGui::Indent(24.f);
                            ImGui::TextUnformatted(label);
                            ImGui::Unindent(24.f);
                            if (hover)
                                GUI::HelpTooltip(label, key.c_str());

                            ImGui::TableNextColumn();


                            // TODO: Hook up the checkbox
                            bool dummyBool = false;
                            ImGui::Checkbox((std::string("##") + key).c_str(), &dummyBool);
                        }
                    }
                }
            }

            ImGui::EndTable();
        }

        // Draw (sub)class header
        bool showHeader = debug || (
            !HiddenClasses.contains(cls->hash)
            && !cls->variables.empty()
            && !(cls->variables.size() == 1 && HoistedVariableSet.contains(cls->variables.begin()->hash))
        );
        if (showHeader)
        {
            if (!ImGui::CollapsingHeader(cls->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                goto BaseClasses;
        }

        if (!StartTable())
            return;

        for (int varNum = 0, varCount = cls->variables.size(); auto& var : cls->variables)
        {
            if (!debug && HoistedVariableSet.contains(var.hash)) {
                varNum++;
                continue;
            }

            BeginRow(var, ent);

            if (debug)
            {
                VarLabel(var.name.c_str(), var.description.c_str(), var.name.c_str());
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

