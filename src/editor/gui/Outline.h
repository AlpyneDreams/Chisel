#pragma once

#include "editor/gui/Layout.h"
#include "entity/Common.h"
#include "entity/components/Transform.h"
#include "imgui.h"
#include "imgui/Window.h"
#include "editor/Selection.h"
#include "entity/Scene.h"
#include "entity/Entity.h"

#include "imgui/IconsMaterialCommunity.h"
#include <string>

namespace engine::editor
{
    struct Outline : public GUI::Window
    {
        Outline() : GUI::Window(ICON_MC_FORMAT_LIST_BULLETED, "Outline", 512, 512, true, ImGuiWindowFlags_MenuBar) {}

        void Draw() override
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu(ICON_MC_PLUS ICON_MC_MENU_DOWN)) {
                    Layout::AddObjectMenu();
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            World.Each([this](auto& ent)
            {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;

                if (Selection.Selected(ent)) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                Entity e = ent;
                const char* name = e.GetName();
                std::string temp;
                if (name == nullptr) {
                    temp = std::string("Entity ") + std::to_string(uint32(ent));
                    name = temp.c_str();
                }

                if (ImGui::TreeNodeEx(&ent, flags, ICON_MC_CUBE_OUTLINE " %s", name))
                {
                    if (ImGui::IsItemClicked())
                    {
                        if (ImGui::GetIO().KeyCtrl) {
                            Selection.Toggle(ent);
                        } else {
                            Selection.Select(ent);
                        }
                    }
                    ImGui::TreePop();
                }
                DrawContextMenu(ent);
            });

            DrawContextMenu();
        }

        void DrawContextMenu(Entity ent = EntityNull)
        {
            auto flags = ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;
            bool window = !ent;
            std::string uid = std::to_string((uintmax_t)ent.handle.entity());
            if (window ? ImGui::BeginPopupContextWindow(NULL, flags) : ImGui::BeginPopupContextItem(uid.c_str()))
            {
                // See Keybinds.h
                if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, !window)) {
                    Editor.Duplicate(ent);
                }
                if (ImGui::MenuItem("Delete", "Delete", false, !window)) {
                    ent.Delete();
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Add")) {
                    Layout::AddObjectMenu();
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
        }
    };
}
