#include "gui/Layout.h"
#include "chisel/Chisel.h"
#include "platform/Platform.h"
#include "console/ConVar.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Modal.h"

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

    void Layout::OpenFilePicker()
    {
        std::string file = Platform.FilePicker();
        if (!file.empty())
            ConCommand::Execute(fmt::format("open_vmf {}", file));
    }

    void Layout::SaveFilePicker()
    {
        std::string file = Platform.FilePicker();
        // TODO
        Chisel.Save();
    }

    void Layout::Update()
    {
        using namespace ImGui;

        bool unsaved = Chisel.HasUnsavedChanges();
        static enum FileAction {
            None, New, Open, Close, Exit, EntityGallery
        } action = None;

        if (BeginMainMenuBar())
        {
            if (BeginMenu("File"))
            {
                if (MenuItem("New",   "Ctrl+N"))                  action = New;
                if (MenuItem("Open",  "Ctrl+O"))                  action = Open;
                if (MenuItem("Save",  "Ctrl+S", unsaved))         Chisel.Save();
                if (MenuItem("Save as...",  "Ctrl+Shift+S"))      SaveFilePicker();
                Separator();
                if (MenuItem("Close", "Ctrl+W"))                  action = Close;
                if (MenuItem("Exit",  "Alt+F4"))                  action = Exit;
                ImGui::EndMenu();
            }

            if (BeginMenu("Map"))
            {
                if (MenuItem("Entity Gallery")) action = EntityGallery;
                ImGui::EndMenu();
            }

            if (BeginMenu("Window"))
            {
                MenuItem(chisel::Tools.console->name.c_str(), "`", &chisel::Tools.console->open);
                MenuItem(Chisel.viewport->name.c_str(), "", &Chisel.viewport->open);
                MenuItem(ICON_MC_APPLICATION_OUTLINE " GUI Demo", "", &gui_demo.value);
                ImGui::EndMenu();
            }

            EndMainMenuBar();
        }

        if (action)
        {
            static bool popup = false;
            if (unsaved && !popup)
            {
                Console.Log("Opening popup...");
                popup = true;
                OpenPopup("Save Changes");
            }

            enum { Waiting = -1, NoPopup, Save = 1, Dont, Cancel };
            int choice = GUI::ModalChoices(
                "Save Changes",
                "Do you want to save the changes you made to the current map?",
                "Save", "Don't Save", "Cancel");

            switch (choice)
            {
                case Waiting: break;
                case Save:    Chisel.Save();
                case Dont:
                case NoPopup:
                    popup = false;
                default:
                {
                    Console.Log("Closing map...");
                    Chisel.CloseMap();
                    switch (action) {
                        case Open: OpenFilePicker(); break;
                        case Exit: Console.Execute("quit"); default: break;
                        case EntityGallery: Chisel.CreateEntityGallery(); break;
                    }
                }
                case Cancel:
                    action = None;
                    popup = false;
            }
        }

        if (BeginViewportSideBar("BottomBar", GetMainViewport(), ImGuiDir_Down, 20.0f, ImGuiWindowFlags_MenuBar))
        {
            if (BeginMenuBar())
            {
                GUI::WindowToggleButton(chisel::Chisel.mainAssetPicker, 64.0f, "Ctrl+Space");
                GUI::WindowToggleButton(chisel::Tools.console, 72.0f, "`");
                EndMenuBar();
            }
            End();
        }

        // ImGuiDockNodeFlags_PassthruCentralNode   - Show viewport under windows. We use a rendertarget instead for 3D views.
        // ImGuiDockNodeFlags_AutoHideTabBar        - Hide tab bar if there is only one window docked in a space.
        DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_AutoHideTabBar);
    }
}
