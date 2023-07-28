#include "TransformTool.h"
#include "chisel/Chisel.h"
#include "chisel/Handles.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Viewport.h"

namespace chisel
{
    using enum TransformType;

    static auto TranslateTool  = TransformTool<Translate>("Translate", ICON_MC_ARROW_ALL, 1);
    static auto RotateTool     = TransformTool<Rotate>("Rotate", ICON_MC_AUTORENEW, 2);
    static auto ScaleTool      = TransformTool<Scale>("Scale", ICON_MC_RESIZE, 3);
    static auto UniversalTool  = TransformTool<Universal>("Transform (All)", ICON_MC_ALPHA_U_BOX_OUTLINE, 4);
    static auto BoundsTool     = TransformTool<Bounds>("Bounds", ICON_MC_VECTOR_SQUARE, 5);

    Tool* Tool::Default        = &TranslateTool;

    template <TransformType Type>
    void TransformTool<Type>::DrawHandles(Viewport& viewport)
    {
        mat4x4 view = viewport.camera.ViewMatrix();
        mat4x4 proj = viewport.camera.ProjMatrix();

        bool snap;
        vec3 size;

        if constexpr (Type == Rotate)
        {
            snap = view_rotate_snap;
            size = vec3(view_rotate_snap_angle);
        }
        else
        {
            snap = view_grid_snap;
            size = view_grid_size;
        }

        std::optional<AABB> bounds = Selection.GetBounds();

        if (!bounds)
            return;

        static bool s_duplicated = false;

        if (!Keyboard.shift || !Mouse.GetButton(MouseButton::Left))
            s_duplicated = false;

        if (auto transform = Handles.Manipulate(bounds.value(), view, proj, Type, Chisel.transformSpace, snap, size))
        {
            if (Keyboard.shift && !s_duplicated)
            {
                Selection.Duplicate();
                s_duplicated = true;
            }

            Selection.Transform(transform.value());
            // TODO: Align to grid fights with the gizmo rn :s
            //brush->GetBrush().AlignToGrid(view_grid_size);
        }
    }
}
