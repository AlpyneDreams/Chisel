#pragma once

#include "gui/Common.h"

namespace chisel
{
    struct Toolbar : GUI::Window
    {
        Toolbar(auto... args) : GUI::Window(
            args...,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar) {}

        bool horizontal = false;

        // Override this
        virtual void DrawToolbar() = 0;

        void Draw() final override
        {
            ImVec2 size = ImGui::GetContentRegionAvail();
            horizontal = size.y <= size.x;

            ImGui::BeginGroup();
            DrawToolbar();
            ImGui::EndGroup();
        }

        void PreDraw() override
        {
            //ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(100.f, 32.f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(horizontal ? 0.f : 8.f, 0.f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.f, 8.f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
        }

        void PostDraw() override
        {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
        }

        bool RadioButton(const char* name, bool selected = false)
        {
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetColorU32(ImGuiCol_ButtonHovered));

            if (horizontal)
                ImGui::SameLine();

            bool clicked = GUI::MenuBarButton(name);

            if (selected)
                ImGui::PopStyleColor();
            return clicked;
        }
    };
}