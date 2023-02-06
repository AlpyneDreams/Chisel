#pragma once

#include "chisel/Chisel.h"

#include "platform/Platform.h"
#include "common/Common.h"
#include "common/System.h"
#include "console/ConVar.h"

#include "imgui.h"
#include "gui/Common.h"

#include "gui/IconsMaterialCommunity.h"

namespace chisel
{
    extern ConVar<bool> gui_demo;

    struct SelectionModeToolbar : GUI::Window
    {
        SelectionModeToolbar() : GUI::Window(
            ICON_MC_CURSOR_DEFAULT, "Select", 500, 32, true,
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar) {}

        void PreDraw() override
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(500.f, 32.f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.f, 8.f));
        }

        void PostDraw() override
        {
            ImGui::PopStyleVar(3);
        }

        void Draw() override
        {
            using enum SelectMode;
            ImGui::BeginMenuBar();
            ImGui::TextUnformatted("Select:");
            ImGui::BeginDisabled();
            RadioButton(ICON_MC_VECTOR_POINT " Vertices", SelectMode(-1));
            RadioButton(ICON_MC_VECTOR_POLYLINE " Edges", SelectMode(-1));
            RadioButton(ICON_MC_VECTOR_SQUARE " Faces", SelectMode(-1));
            ImGui::EndDisabled();
            RadioButton(ICON_MC_VECTOR_TRIANGLE " Solids", Solids);
            RadioButton(ICON_MC_CUBE_OUTLINE " Objects", Objects);
            RadioButton(ICON_MC_GROUP " Groups", Groups);
            ImGui::EndMenuBar();
        }

        void RadioButton(const char* name, SelectMode mode)
        {
            bool selected = Chisel.selectMode == mode;

            if (selected)
                ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetColorU32(ImGuiCol_ButtonHovered));

            if (bool clicked = GUI::MenuBarButton(name))
                Chisel.selectMode = mode;

            if (selected)
                ImGui::PopStyleColor();
        }
    };

    struct Layout : System
    {
        Space space = Space::World;

        void Start() override
        {
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = "chisel.ui.ini";
        }

        static void OpenFilePicker()
        {
            std::string file = Platform.FilePicker();
            if (!file.empty())
                ConCommand::Execute(fmt::format("open_vmf {}", file));
        }

        void Update() override
        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New")) {

                    }
                    if (ImGui::MenuItem("Open", "Ctrl+O")) {
                        OpenFilePicker();
                    }
                    if (ImGui::MenuItem("Quit")) {
                        Console.Execute("quit");
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Window"))
                {
                    ImGui::MenuItem(chisel::Tools.console->name.c_str(), "`", &chisel::Tools.console->open);
                    ImGui::MenuItem(Chisel.viewport->name.c_str(), "", &Chisel.viewport->open);
                    ImGui::MenuItem(ICON_MC_APPLICATION_OUTLINE " GUI Demo", "", &gui_demo.value);
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            if (ImGui::BeginViewportSideBar("BottomBar", ImGui::GetMainViewport(), ImGuiDir_Down, 20.0f, ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::BeginMenuBar())
                {
                    GUI::WindowToggleButton(chisel::Tools.console, 72.0f, "`");
                    ImGui::EndMenuBar();
                }
                ImGui::End();
            }

            // ImGuiDockNodeFlags_PassthruCentralNode   - Show viewport under windows. We use a rendertarget instead for 3D views.
            // ImGuiDockNodeFlags_AutoHideTabBar        - Hide tab bar if there is only one window docked in a space.
            ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_AutoHideTabBar);
        }
    };
}
