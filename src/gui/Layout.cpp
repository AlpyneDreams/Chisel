#include "gui/Layout.h"
#include "chisel/Chisel.h"
#include "platform/Platform.h"
#include "console/ConVar.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Modal.h"
#include "gui/Viewport.h"
#include "input/Mouse.h"

#include <glm/gtc/round.hpp>

namespace chisel
{
    ConVar gui_demo("gui_demo", false, "Show ImGui demo window");

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
    //  Edit Toolbar
    //--------------------------------------------------

    EditingToolbar::EditingToolbar() : Toolbar(
        ICON_MC_CURSOR_DEFAULT, "Editing", 500, 32, true) {}

    void EditingToolbar::DrawToolbar()
    {
        const char* name = "";
        switch (Chisel.transformSpace) {
            default:
            case Space::World: name = ICON_MC_WEB " World"; break;
            case Space::Local: name = ICON_MC_CUBE_OUTLINE " Local"; break;
        }
        if (RadioButton(name)) {
            if (Chisel.transformSpace == Space::World)
                Chisel.transformSpace = Space::Local;
            else
                Chisel.transformSpace = Space::World;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Transform Mode");

        RadioButton(ICON_MC_AXIS_ARROW " Axis Flip", &view_axis_allow_flip.value);

        RadioButton(
            trans_texture_lock
                ? (ICON_MC_TEXTURE " " ICON_MC_LOCK)
                : (ICON_MC_TEXTURE " " ICON_MC_LOCK_OPEN_OUTLINE),
            &trans_texture_lock.value,
            "Texture Lock"
        );

        RadioButton(
            trans_texture_scale_lock
                ? (ICON_MC_TEXTURE ICON_MC_ARROW_LEFT_RIGHT " " ICON_MC_LOCK)
                : (ICON_MC_TEXTURE ICON_MC_ARROW_LEFT_RIGHT " " ICON_MC_LOCK_OPEN_OUTLINE),
            &trans_texture_scale_lock.value,
            "Texture Scale Lock"
        );

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));

        // Grid and snap settings
        {
            RadioButton(
                view_grid_show ? ICON_MC_GRID : ICON_MC_GRID_OFF,
                &view_grid_show.value,
                "Show Grid"
            );

            RadioButton(ICON_MC_MAGNET, &view_grid_snap.value, "Snap to Grid");

            // + and - buttons
            ImGui::SameLine();
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.5f));
            ImGui::BeginGroup();
            {
                if (ImGui::Button(ICON_MC_CHEVRON_UP))
                    view_grid_size.value *= 2;
                if (ImGui::IsItemHovered())
                    GUI::ShortcutTooltip("Increase Grid Size    ", "]");

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 7.f);
                if (ImGui::Button(ICON_MC_CHEVRON_DOWN))
                    view_grid_size.value /= 2;
                if (ImGui::IsItemHovered())
                    GUI::ShortcutTooltip("Decrease Grid Size    ", "[");
            }
            ImGui::EndGroup();
            ImGui::PopStyleVar(1); // FramePadding
            
            ImGui::SameLine();
            ImGui::SetNextItemWidth(58.f);
            if (ImGui::BeginCombo("##GridOptions", str::format("%4g " ICON_MC_MENU_DOWN, view_grid_size.value.x).c_str(), 
                ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoArrowButton))
            {
                ImGui::PopStyleVar(2);   // ItemSpacing, FrameRounding
                ImGui::PopStyleColor(2); // Button, FrameBg
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultPadding);
                ImGui::TextUnformatted("Grid");
                ImGui::Separator();

                if (ImGui::Button(uniformGridSize ? ICON_MC_LINK : ICON_MC_LINK_OFF))
                    uniformGridSize = !uniformGridSize;
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Uniform Grid Size");

                ImGui::SameLine();
                ImGui::SetNextItemWidth(3 * 42.f);
                if (uniformGridSize) {
                    if (ImGui::DragFloat("##GridSize1", &view_grid_size.value.x, 1.f, 0.125f, 1024.f, "%g"))
                        view_grid_size = view_grid_size.value.xxx;
                } else {
                    ImGui::InputFloat3("##GridSize3", &view_grid_size.value.x, "%g");
                }
                
                const float gridSizes[] = { 
                    0.125f, 0.25f, 0.5f, 1.f, 2.f, 4.f, 8.f,
                    16.f, 32.f, 64.f, 128.f, 256.f, 512.f, 1024.f
                };

                for (auto size : gridSizes) {
                    bool selected = view_grid_size.value.x == size;
                    if (ImGui::Selectable(str::format("%g", size).c_str(), selected))
                        view_grid_size = vec3(size);
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::Checkbox("Snap Hit Surface to Grid", &view_grid_snap_hit_normal.value);

                ImGui::EndCombo();
                ImGui::PopStyleVar(); // FramePadding
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
            }
            else ImGui::PopStyleVar(2); // ItemSpacing, FrameRounding

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Grid Options");
        }

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        
        // Angle snap settings
        {

            const char* rotationIcon = view_rotate_snap_angle < 90.f ? ICON_MC_ANGLE_ACUTE
                                    : (view_rotate_snap_angle == 90.f ? ICON_MC_ANGLE_RIGHT : ICON_MC_ANGLE_OBTUSE);

            ImGui::SameLine();
            RadioButton(rotationIcon, &view_rotate_snap.value, "Rotation Snap");

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);

            ImGui::SameLine();
            ImGui::SetNextItemWidth(50.f);
            if (ImGui::BeginCombo("##AngleSnap", str::format("%3g° " ICON_MC_MENU_DOWN, view_rotate_snap_angle.value).c_str(),
                ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoArrowButton))
            {
                ImGui::PopStyleVar(1);      // FrameRounding
                ImGui::PopStyleColor(2);    // Button, FrameBg
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultPadding);
                ImGui::TextUnformatted("Rotation");
                ImGui::Separator();

                ImGui::SetNextItemWidth(64.f);
                ImGui::DragFloat("##AngleSnap1", &view_rotate_snap_angle.value, 1.f, 1.f, 180.f, "%g°");

                const float angleSnaps[] = { 
                    1.f, 5.f, 15.f, 30.f, 45.f, 90.f
                };

                for (auto angle : angleSnaps) {
                    bool selected = view_rotate_snap_angle.value == angle;
                    if (ImGui::Selectable(str::format("%g°", angle).c_str(), selected))
                        view_rotate_snap_angle = angle;
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
                ImGui::PopStyleVar(); // FramePadding
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
            }
            else ImGui::PopStyleVar(1); // FrameRounding

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Rotation Snap");
        }
        ImGui::PopStyleColor(2); // Button, FrameBg

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
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
        Option(ICON_MC_SQUARE_OFF_OUTLINE, "Clip", Tool::Clip);
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

    static constexpr std::array<ExtensionName, 4> MapExtensionsAndAll =
    { {
        { "All Map Files", "*.VMF,*.BOX,*.MAP" },
        { "Boxcutter Map File", "*.BOX" },
        { "Valve Map File", "*.VMF" },
        { "Quake-style", "*.MAP" },
    } };

    static constexpr std::array<ExtensionName, 3> MapExtensions =
    {{
        { "Boxcutter Map File", "*.BOX" },
        { "Valve Map File", "*.VMF" },
        { "Quake-style", "*.MAP" },
    }};

    void Layout::OpenFilePicker()
    {
        std::string file = Platform.FilePicker(true, MapExtensionsAndAll);
        if (!file.empty())
            ConCommand::Execute(fmt::format("open_map {}", file));
    }

    void Layout::SaveFilePicker()
    {
        std::string file = Platform.FilePicker(false, MapExtensions);
        // TODO
        Chisel.Save(file);
    }

    void Layout::Update()
    {
        using namespace ImGui;

        if (gui_demo) {
            ShowDemoWindow();
        }

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
                //if (MenuItem("Save",  "Ctrl+S", unsaved))         Chisel.Save();
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
                MenuItem(Chisel.console->name.c_str(), "`", &Chisel.console->open);

                if (MenuItem(ICON_MC_IMAGE_PLUS " Add Viewport"))
                {
                    Tools.systems.AddSystem<Viewport>();
                }

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
                    // TODO: Hook up active map path
                //case Save:    Chisel.Save();
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
                GUI::WindowToggleButton(Chisel.mainAssetPicker, 64.0f, "Ctrl+Space");
                GUI::WindowToggleButton(Chisel.console, 72.0f, "`");
                EndMenuBar();
            }
            End();
        }

        // ImGuiDockNodeFlags_PassthruCentralNode   - Show viewport under windows. We use a rendertarget instead for 3D views.
        // ImGuiDockNodeFlags_AutoHideTabBar        - Hide tab bar if there is only one window docked in a space.
        DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_AutoHideTabBar);
    }
}
