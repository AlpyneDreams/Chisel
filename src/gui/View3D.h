#pragma once

#include "gui/Window.h"
#include "console/ConVar.h"
#include "core/Camera.h"
#include "render/Render.h"

namespace chisel
{
    inline ConVar<bool>  view_axis_allow_flip("view_axis_allow_flip", false, "Allow gizmos to flip axes contextually.");
    inline ConVar<bool>  view_grid_show("view_grid_show", true, "Show grid.");
    inline ConVar<bool>  view_grid_snap("view_grid_snap", true, "Snap to grid.");
    inline ConVar<vec3>  view_grid_size("view_grid_size", vec3(64.f), "Grid snap size.");
    inline ConVar<bool>  view_grid_snap_hit_normal("view_grid_snap_hit_normal", false, "Should the axis of the normal of the raycast also be snapped?");
    inline ConVar<bool>  view_rotate_snap("view_rotate_snap", true, "Snap rotation angles.");
    inline ConVar<float>  view_rotate_snap_angle("view_rotate_snap_angle", 15.f, "Snap rotation angles.");

    inline ConVar<bool>  trans_texture_lock("trans_texture_lock", true, "Enable texture lock for transformations.");
    inline ConVar<bool>  trans_texture_scale_lock("trans_texture_scale_lock", false, "Enable scaling texture lock.");
    inline ConVar<bool>  trans_texture_face_alignment("trans_texture_face_alignment", true, "Enable texture face alignment.");

    struct View3D : public GUI::Window
    {
        View3D(auto... args) : GUI::Window(args...) { }

        Camera camera;

        Rect  viewport;

        bool  gridUniform    = true;

        bool  mouseOver      = false;
        bool  popupOpen      = false;

    // Rendering //
        virtual void  Render() = 0;
        virtual void* GetMainTexture() = 0;

    // Virtual Methods //

        virtual void Start() override;
        virtual void OnClick(uint2 pos) {}
        virtual void OnResize(uint width, uint height) {}
        virtual void DrawHandles(mat4x4& view, mat4x4& proj) {}
        virtual void OnDrawMenu() {}
        virtual void OnPostDraw() {}

        Camera& GetCamera();
        uint2 GetMousePos() const;
        Ray GetMouseRay();

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
    };
}
