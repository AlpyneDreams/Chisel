#include "gui/Layout.h"
#include "chisel/Chisel.h"
#include "platform/Platform.h"
#include "console/ConVar.h"
#include "gui/IconsMaterialCommunity.h"

namespace chisel
{
    extern ConVar<bool> gui_demo;

    //--------------------------------------------------
    //  Selection Mode Toolbar
    //--------------------------------------------------

    SelectionModeToolbar::SelectionModeToolbar() : Toolbar(
        ICON_MC_CURSOR_DEFAULT, "Select", 500, 32, true) {}

    void SelectionModeToolbar::DrawToolbar()
    {
        using enum SelectMode;
        ImGui::BeginDisabled();
        Option("Select:", SelectMode(-1));
        Option(ICON_MC_VECTOR_POINT " Vertices", SelectMode(-1));
        Option(ICON_MC_VECTOR_POLYLINE " Edges", SelectMode(-1));
        Option(ICON_MC_VECTOR_SQUARE " Faces", SelectMode(-1));
        ImGui::EndDisabled();
        Option(ICON_MC_VECTOR_TRIANGLE " Solids", Solids);
        Option(ICON_MC_CUBE_OUTLINE " Objects", Objects);
        Option(ICON_MC_GROUP " Groups", Groups);
    }

    void SelectionModeToolbar::Option(const char* name, SelectMode mode)
    {
        bool selected = Chisel.selectMode == mode;

        if (RadioButton(name, selected))
            Chisel.selectMode = mode;
    }

    //--------------------------------------------------
    //  Main Toolbar
    //--------------------------------------------------

    MainToolbar::MainToolbar() : Toolbar(
        ICON_MC_CURSOR_DEFAULT, "Tools", 39, 500, true) {}

    void MainToolbar::DrawToolbar()
    {
        ImGui::Spacing();

        Option(ICON_MC_CURSOR_DEFAULT, "Select", Tool::Select);
        Option(ICON_MC_ARROW_ALL, "Translate", Tool::Translate);
        Option(ICON_MC_AUTORENEW, "Rotate", Tool::Rotate);
        Option(ICON_MC_RESIZE, "Scale", Tool::Scale);
        Option(ICON_MC_ALPHA_U_BOX_OUTLINE, "Transform (All)", Tool::Universal);
        Option(ICON_MC_VECTOR_SQUARE, "Bounds", Tool::Bounds);
        ImGui::Separator();
        Option(ICON_MC_LIGHTBULB, "Entity", Tool::Entity);
        Option(ICON_MC_CUBE_OUTLINE, "Block", Tool::Block);
    }

    void MainToolbar::Option(const char* icon, const char* name, Tool tool)
    {
        ImGuiCol col = Chisel.activeTool == tool ? ImGuiCol_TabActive : ImGuiCol_WindowBg;
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(col));

        if (ImGui::Button(icon)) {
            Chisel.activeTool = tool;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(name);
        }

        ImGui::PopStyleColor();
    }

    //--------------------------------------------------
    //  Layout
    //--------------------------------------------------

    void Layout::Start()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = "chisel.ui.ini";
    }

    void Layout::OpenFilePicker()
    {
        std::string file = Platform.FilePicker();
        if (!file.empty())
            ConCommand::Execute(fmt::format("open_vmf {}", file));
    }

    void Layout::Update()
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
}
