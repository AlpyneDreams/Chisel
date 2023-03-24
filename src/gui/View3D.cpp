#include "gui/View3D.h"
#include "chisel/Tools.h"
#include "chisel/Selection.h"
#include "chisel/Chisel.h"
#include "input/Input.h"
#include "input/Keyboard.h"
#include "platform/Cursor.h"
#include "common/Time.h"

#include "core/Transform.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include "gui/IconsMaterialCommunity.h"
#include "render/Render.h"

namespace chisel
{
    inline ConVar<float> cam_maxspeed ("cam_maxspeed",  750.0f,  "Max speed");
    inline ConVar<float> cam_pitchup  ("cam_pitchup",   +89.0f,  "Set the max pitch value.");
    inline ConVar<float> cam_pitchdown("cam_pitchdown", -89.0f,  "Set the min pitch value.");

    inline ConVar<float> m_sensitivity("m_sensitivity", 6.0f,   "Mouse sensitivity");
    inline ConVar<float> m_pitch      ("m_pitch",       0.022f, "Mouse pitch factor.");
    inline ConVar<float> m_yaw        ("m_yaw",         0.022f, "Mouse yaw factor.");

    Camera& View3D::GetCamera() { return Tools.editorCamera.camera; }

// Draw Modes //

    enum class DrawMode {
        Shaded, Depth
    };

    static inline const char* drawModes[] = { "Shaded", "Depth" };

    DrawMode drawMode = DrawMode::Shaded;

    auto View3D::GetTexture(DrawMode mode)
    {
        switch (mode) {
            default: case DrawMode::Shaded:
                return Tools.rt_SceneView->GetTexture();
            case DrawMode::Depth:
                return Tools.rt_SceneView->GetDepthTexture();
        }
    }

// UI //

    bool View3D::BeginMenu(const char* label, const char* name, bool enabled)
    {
        std::string menuName = std::string(label) + "###" + name;
        bool open = ImGui::BeginMenu(menuName.c_str(), enabled);

        if (!open && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", name);

        if (open)
        {
            ImGui::TextUnformatted(name);
            ImGui::Separator();
        }

        // Prevent click on viewport when clicking menu
        if (open && ImGui::IsWindowHovered())
            popupOpen = true;
        return open;
    }

    void View3D::NoPadding() {
        // Set window padding to 0
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }

    void View3D::ResetPadding() {
        ImGui::PopStyleVar();
    }

    void View3D::PreDraw()
    {
        NoPadding();
    }

    void View3D::PostDraw()
    {
        ResetPadding();

        if (!visible)
            return;

        // HACK: Set hovered window to NULL,
        // this fixes mouse over with ImGui docking
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiWindow* hovered = g.HoveredWindow;
        g.HoveredWindow = NULL;

        Handles.Begin(viewport, view_axis_allow_flip);

        Camera& camera = GetCamera();

        // Get camera matrices
        mat4x4 view = camera.ViewMatrix();
        mat4x4 proj = camera.ProjMatrix();

        DrawHandles(view, proj);

        // HACK: Reset hovered window
        g.HoveredWindow = hovered;

        // Begin scene view extra rendering
        render::Render& r = Tools.Render;
        r.SetRenderTarget(Tools.rt_SceneView);

        // Draw grid
        if (view_grid_show)
            Handles.DrawGrid(r, camera.position, Tools.sh_Grid, gridSize);

        OnPostDraw();
    }

    void View3D::Draw()
    {
        popupOpen = false;
        ResetPadding();

    // Menu Bar //
        if (ImGui::BeginMenuBar())
        {
            // Left side
            CoordinateSpacePicker();
            GridMenu();

            const char* rotationIcon = rotationSnap < 90.f ? ICON_MC_ANGLE_ACUTE
                                        : (rotationSnap == 90.f ? ICON_MC_ANGLE_RIGHT : ICON_MC_ANGLE_OBTUSE);
            if (BeginMenu(str::format("%s %g " ICON_MC_MENU_DOWN, rotationIcon, rotationSnap).c_str(), "Rotation"))
            {
                ImGui::Checkbox(ICON_MC_MAGNET " Rotation Snap", &view_rotate_snap.value);
                ImGui::InputFloat("Angle Snap", &rotationSnap);
                ImGui::EndMenu();
            }

            // Right side
            ImGui::SameLine(ImGui::GetWindowWidth() - 90);
            if (BeginMenu(ICON_MC_IMAGE_MULTIPLE " " ICON_MC_MENU_DOWN, "Render Mode"))
            {
                for (int i = 0; i < sizeof(drawModes) / sizeof(const char*); i++)
                {
                    if (ImGui::MenuItem(drawModes[i], "", drawMode == DrawMode(i)))
                        drawMode = DrawMode(i);
                }
                ImGui::EndMenu();
            }

            // Right side
            ImGui::SameLine(ImGui::GetWindowWidth() - 40);
            if (BeginMenu(ICON_MC_VIDEO " " ICON_MC_MENU_DOWN, "Camera"))
            {
                Camera& camera = GetCamera();
                ImGui::InputFloat("FOV", &camera.fieldOfView);
                ImGui::InputFloat("Speed (hu/s)", &cam_maxspeed.value);
                ImGui::InputFloat("Sensitivity", &m_sensitivity.value);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        NoPadding();

        // Return if window is collapsed
        if (!CheckResize()) {
            return;
        }

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetContentRegionAvail();
        ImVec2 max = ImVec2(pos.x + size.x, pos.y + size.y);

        // Copy from scene view render target into viewport
        ImGui::GetWindowDrawList()->AddImage(
            GetTexture(drawMode),
            pos, max,
            ImVec2(0, 0), ImVec2(1, 1)
        );

        viewport = Rect(pos.x, pos.y, size.x, size.y);

        // If mouse is over viewport,
        if (ImGui::IsWindowHovered() && IsMouseOver(viewport))
        {
            Camera& camera = GetCamera();

            // Left-click: Select (or transform selection)
            if (Mouse.GetButtonDown(Mouse::Left) && !popupOpen && !Handles.IsMouseOver())
            {
                ImVec2 absolute = ImGui::GetMousePos();
                uint2 mouse = uint2(absolute.x - pos.x, absolute.y - pos.y);
                OnClick(mouse);
            }

            if (!ImGui::GetIO().KeyCtrl)
            {
                // Right-click and hold (or press Z) to mouselook
                // TODO: Make Z toggle instead of hold
                if (Mouse.GetButtonDown(Mouse::Right) || Keyboard.GetKeyDown(Key::Z))
                {
                    Cursor.SetMode(Cursor::Locked);
                    Cursor.SetVisible(false);
                }

                if (Mouse.GetButton(Mouse::Right) || Keyboard.GetKey(Key::Z))
                {
                    MouseLook(Mouse.GetMotion());
                }

                if (Mouse.GetButtonUp(Mouse::Right) || Keyboard.GetKeyUp(Key::Z))
                {
                    Cursor.SetMode(Cursor::Normal);
                    Cursor.SetVisible(true);
                }

                // WASD. TODO: Virtual axes, arrow keys
                float w = Keyboard.GetKey(Key::W) ? 1.f : 0.f;
                float s = Keyboard.GetKey(Key::S) ? 1.f : 0.f;
                float a = Keyboard.GetKey(Key::A) ? 1.f : 0.f;
                float d = Keyboard.GetKey(Key::D) ? 1.f : 0.f;

                camera.position += camera.Forward() * (w-s) * cam_maxspeed.value * float(Time.deltaTime);
                camera.position += camera.Right() * (d-a) * cam_maxspeed.value * float(Time.deltaTime);
            }
        }
    }
    
    void View3D::MouseLook(int2 mouse)
    {
        Camera& camera = GetCamera();

        // Pitch down, yaw left
        if (camera.rightHanded)
            mouse = -mouse;

        // Apply mouselook
        vec3 angles = math::degrees(camera.angles);
        angles.xy += vec2(mouse.y * m_pitch, mouse.x * m_yaw) * float(m_sensitivity);

        // Clamp up/down look angles
        angles.x = std::clamp<float>(angles.x, cam_pitchdown, cam_pitchup);

        camera.angles = math::radians(angles);
    }

    // Returns true if window is not collapsed
    bool View3D::CheckResize()
    {
        // If window resized: resize render target
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x != width || size.y != height)
        {
            // Window is collapsed or too small
            if (size.x == 0 || size.y == 0) {
                return false;
            }

            width = uint(size.x);
            height = uint(size.y);

            // Resize framebuffer or viewport
            Tools.ResizeViewport(width, height);

            return true;
        }

        return true;
    }

// Coordinate Space Picker //

    void View3D::CoordinateSpacePicker()
    {
        const char* items[] = {
            ICON_MC_WEB " World",
            ICON_MC_CUBE_OUTLINE " Local"
        };
        int i = int(space);
        auto label = std::string(items[i]) + " " ICON_MC_MENU_DOWN;
        if (BeginMenu(label.c_str(), "Transform")) {
            for (size_t j = 0; j < std::size(items); j++) {
                if (ImGui::MenuItem(items[j], "", space == Space(j))) {
                    space = Space(j);
                }
            }
            ImGui::Checkbox("Axis Flip", &view_axis_allow_flip.value);
            ImGui::EndMenu();
        }
    }

// Grid Menu //

    void View3D::GridMenu()
    {
        char gridTabName[128];
        const bool isGridUniform = gridUniform || (gridSize.x == gridSize.y && gridSize.y == gridSize.z);
        if (isGridUniform)
            snprintf(gridTabName, sizeof(gridTabName), ICON_MC_GRID " %g " ICON_MC_MENU_DOWN, gridSize.x);
        else
            snprintf(gridTabName, sizeof(gridTabName), ICON_MC_GRID " %g %g %g " ICON_MC_MENU_DOWN, gridSize.x, gridSize.y, gridSize.z);
        if (BeginMenu(gridTabName, "Grid"))
        {
            const char* label = view_grid_show
                ? (ICON_MC_GRID " Show Grid")
                : (ICON_MC_GRID_OFF " Show Grid");
            ImGui::Checkbox(label, &view_grid_show.value);
            ImGui::Checkbox(ICON_MC_MAGNET " Snap to Grid", &view_grid_snap.value);
            ImGui::Checkbox(ICON_MC_LINK " Uniform Grid Size", &gridUniform);
            if (gridUniform) {
                if (ImGui::InputFloat("Grid Size", &gridSize.x))
                    OnResizeGrid(gridSize = gridSize.xxx);
            } else {
                if (ImGui::InputFloat3("Grid Size", &gridSize.x))
                    OnResizeGrid(gridSize);
            }
            ImGui::EndMenu();
        }
    }
}
