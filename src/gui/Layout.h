#pragma once

#include "chisel/Chisel.h"

#include "platform/Platform.h"
#include "common/Common.h"
#include "core/System.h"
#include "console/ConVar.h"

#include "imgui.h"
#include "gui/Common.h"

#include "gui/IconsMaterialCommunity.h"

namespace chisel {
    extern ConVar<bool> gui_demo;
}

namespace chisel
{
    struct SelectionModeWindow : GUI::Window
    {
        SelectionModeWindow() : GUI::Window(ICON_MC_CURSOR_DEFAULT, "Select", 256, 256, true) {}

        void Draw() override
        {
            using enum SelectMode;

            RadioButton("Groups", Groups);
            RadioButton("Objects", Objects);
            RadioButton("Solids", Solids);
        }

        void RadioButton(const char* name, SelectMode mode)
        {
            bool selected = Chisel.selectMode == mode;

            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonHovered));

            if (ImGui::Button(name, ImVec2(ImGui::GetContentRegionAvail().x, 0.f)))
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
            if (!file.empty()) {
                Chisel.Open(file.c_str());
            }
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

            // ImGuiDockNodeFlags_PassthruCentralNode
            ImGui::DockSpaceOverViewport(NULL);
        }
    };
}
