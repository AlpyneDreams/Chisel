#pragma once

#include "common/Common.h"
#include "common/Reflection.h"
#include "entity/Common.h"
#include "imgui.h"
#include "imgui/IconsMaterialCommunity.h"
#include "imgui/Window.h"
#include "editor/Selection.h"
#include "editor/gui/Layout.h"
#include "entity/Entity.h"
#include "entity/components/Name.h"
#include "entity/components/Transform.h"
#include "imgui/Common.h"
#include "editor/Icons.h"

#include "console/Console.h"
#include "math/Math.h"

#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <type_traits>

namespace engine::editor
{
    struct Inspector : public GUI::Window
    {
        Inspector() : GUI::Window(ICON_MC_INFORMATION, "Inspector", 512, 512, true, ImGuiWindowFlags_MenuBar) {}

        bool debug = false;
        bool locked = false;

        Entity target = EntityNull;

        void Draw() override
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
                target = Selection.Active();

            Entity ent = target;
            if (!ent) {
                locked = false;
                return;
            }

            // Get entity name, if any
            const char* name = ent.GetName();
            std::string buf;
            if (name != nullptr)
                buf = name;
            
            // Draw entity name field
            GUI::ItemLabel("Name");
            if (ImGui::InputTextWithHint("##Name", "Entity Name", &buf)) {
                ent.SetName(buf);
            }

            for (auto&& [id, component] : ent.GetComponents())
            {
                using namespace refl;

                Class* cls = Class::Get(id);

                if (!cls) {
                    ImGui::Text("Component: %u", id);
                    continue;
                }

                // Internal component. Handled above.
                if (cls->type.hash == TypeHash<Name> && !debug) {
                    continue;
                }

                // Draw component inspector, remove component if removed
                if (!DrawComponentInspector(*cls, ent, component))
                {
                    ent.RemoveComponent(id);
                    continue;
                }
            }
            
            if (ImGui::Button(ICON_MC_PLUS " Add Component")) {
                ImGui::OpenPopup("AddComponentList");
            }

            if (ImGui::BeginPopup("AddComponentList")) {
                Layout::AddComponentMenu(ent);
                ImGui::EndPopup();
            }
        }

        bool DrawComponentInspector(refl::Class& component, Entity& ent, Component* obj)
        {
            using namespace refl;

            // Create a unique ID for each component on each entity version
            // so that every unique component has its own collapsing header.
            auto id = std::to_string((uintmax_t)ent.handle.entity());
            auto ver = std::to_string(ent.handle.registry()->current(ent));
            auto label = std::string(component.displayName) + "##" + id + "/" + ver;

            if (ComponentIcons.contains(component.type.hash)) {
                label = ComponentIcons[component.type.hash] + " " + label;
            }

            bool visible = true;
            bool expanded = ImGui::CollapsingHeader(label.c_str(), &visible, ImGuiTreeNodeFlags_DefaultOpen);

            // X button pressed - remove component
            if (!visible) {
                ent.RemoveComponent(component.type.hash);
                return false;
            }

            if (!expanded)
                return true;

            // In debug mode, draw all components' raw values
            if (debug) {
                DrawComponent(component, ent, obj);
                return true;
            }

            // TODO: Custom inspector registration system
            switch (component.type.hash)
            {
                case TypeHash<Transform>:
                    Inspect((Transform*)obj);
                    break;
                default:
                    DrawComponent(component, ent, obj);
                    break;
            }
            
            return true;
        }
    
    protected:

        // Default Inspector
        void DrawComponent(refl::Class& component, Entity& ent, void* obj)
        {
            using namespace refl;
            for (Field& field : component.fields) {
                ImGui::PushID(&field);
                Input(field.displayName, field.type.hash, field.GetPointer<void>(obj));
                ImGui::PopID();
            }
        }

        void Inspect(auto* t) { Inspect(*t); }

        // Basic Custom Inspectors

        void Inspect(Transform& t)
        {
            Input("Position", t.position);

            auto euler = t.GetEulerAngles();
            if (Input("Rotation", euler)) {
                t.SetEulerAngles(euler);
            }

            Input("Scale", t.scale);
        }
    public:

        static bool Input(const char* name, auto& ref)
        {
            return Input(name, refl::TypeHash<std::remove_reference_t<decltype(ref)>>, &ref);
        }

        static bool Input(const char* name, refl::Hash type, void* ptr)
        {
            using namespace refl;

            GUI::ItemLabel(name);

            #define INPUT_POINTER(Type, Value, Input) \
                case TypeHash<Type>:                  \
                    return ImGui::Input(name, (Value*)ptr);
            
            #define INPUT_TYPE(Type, Input) INPUT_POINTER(Type, Type, Input)

            #define INPUT_SCALAR(Type, Scalar) \
                case TypeHash<Type>:           \
                    return ImGui::InputScalar(name, ImGuiDataType_ ## Scalar, (Type*)ptr);
            
            switch (type)
            {
                case TypeHash<bool>:
                    return ImGui::Checkbox((std::string("##") + name).c_str(), (bool*)ptr);
                
                INPUT_TYPE(float, InputFloat);
                INPUT_TYPE(double, InputDouble);
                INPUT_TYPE(std::string, InputText);
                INPUT_SCALAR(int8,   S8);
                INPUT_SCALAR(uint8,  U8);
                INPUT_SCALAR(int16,  S16);
                INPUT_SCALAR(uint16, U16);
                INPUT_SCALAR(int32,  S32);
                INPUT_SCALAR(uint32, U32);
                INPUT_SCALAR(int64,  S64);
                INPUT_SCALAR(uint64, U64);
                INPUT_POINTER(vec4, float, InputFloat4);
                INPUT_POINTER(vec3, float, InputFloat3);
                INPUT_POINTER(vec2, float, InputFloat2);
                INPUT_POINTER(int4, int, InputInt4);
                INPUT_POINTER(int3, int, InputInt3);
                INPUT_POINTER(int2, int, InputInt2);


                case TypeHash<quat>: {
                    quat* q = (quat*)ptr;
                    // TODO: Improve euler -> quaternion -> euler roundtrip
                    vec3 angles = glm::eulerAngles(*q);
                    vec3 d = glm::degrees(angles);
                    if (ImGui::InputFloat3(name, &d[0])) {
                        *q = quat(glm::radians(d));
                        return true;
                    }
                    return false;
                }
                case TypeHash<Rect>: {
                    Rect* rect = (Rect*)ptr;

                    bool changed = false;
                    if (ImGui::BeginTable(name, 2))
                    {
                        ImGui::TableNextColumn();
                        changed |= ImGui::InputFloat("X", &rect->x);
                        ImGui::TableNextColumn();
                        changed |= ImGui::InputFloat("Y", &rect->y);
                        ImGui::TableNextColumn();
                        changed |= ImGui::InputFloat("W", &rect->w);
                        ImGui::TableNextColumn();
                        changed |= ImGui::InputFloat("H", &rect->h);

                        ImGui::EndTable();
                    }

                    return changed;
                }
                default: {
                    ImGui::Text("%s", name);
                    return false;
                }
            }
        }
    };
}
