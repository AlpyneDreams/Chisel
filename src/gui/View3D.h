#pragma once

#include "gui/Window.h"
#include "chisel/Handles.h"
#include "console/ConVar.h"
#include "core/Camera.h"

namespace chisel
{
    inline ConVar<bool>  view_axis_allow_flip("view_axis_allow_flip", false, "Allow gizmos to flip axes contextually.");
    inline ConVar<bool>  view_grid_show("view_grid_show", true, "Show grid.");
    inline ConVar<bool>  view_grid_snap("view_grid_snap", true, "Snap to grid.");
    inline ConVar<bool>  view_rotate_snap("view_rotate_snap", true, "Snap rotation angles.");

    struct View3D : public GUI::Window
    {
        View3D(auto... args) : GUI::Window(args..., ImGuiWindowFlags_MenuBar) {}

        Space space          = Space::World;
        Rect  viewport;

        vec3  gridSize       = vec3(64.f);
        bool  gridUniform    = true;
        float rotationSnap   = 15.f;

        bool  mouseOver      = false;
        bool  popupOpen      = false;

    // Virtual Methods //

        virtual void OnClick(uint2 pos) {}
        virtual void OnResizeGrid(vec3& gridSize) {}
        virtual void DrawHandles(mat4x4& view, mat4x4& proj) {}
        virtual void OnPostDraw() {}

        Camera& GetCamera() const;
        uint2 GetMousePos() const;
        Ray GetMouseRay() const;

    // Draw Modes //

        enum class DrawMode {
            Shaded, Depth
        };

        static inline const char* drawModes[] = { "Shaded", "Depth" };

        DrawMode drawMode = DrawMode::Shaded;

        ID3D11ShaderResourceView* GetTexture(DrawMode mode);

    // UI //

        bool BeginMenu(const char* label, const char* name, bool enabled = true);

        void NoPadding();

        void ResetPadding();

        void PreDraw();
        void Draw() override;
        void PostDraw();

    protected:
        void MouseLook(int2 mouse);

        // Returns true if window is not collapsed
        bool CheckResize();

        void CoordinateSpacePicker();

        void GridMenu();

        void Toolbar();
        void ToolbarButton(const char* label, Tool tool);

    };
}
