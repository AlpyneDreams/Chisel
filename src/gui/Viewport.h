#include "gui/View3D.h"

#include "chisel/Chisel.h"
#include "chisel/Gizmos.h"

namespace chisel
{
    using chisel::View3D;

    struct Viewport : public View3D
    {
        Viewport() : View3D(ICON_MC_IMAGE_SIZE_SELECT_ACTUAL, "Viewport", 512, 512, true) {}

        void DrawHandles(mat4x4& view, mat4x4& proj) override
        {
            Chisel.Renderer->DrawHandles(view, proj, activeTool, space, view_grid_snap, gridSize);
        }

        void OnPostDraw() override
        {
            // TODO: Better way to find solid by ID
            //Gizmos.DrawIcon(vec3(0), Gizmos.icnLight);
        }

    };
}
