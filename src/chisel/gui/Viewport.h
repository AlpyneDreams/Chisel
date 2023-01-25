#include "gui/View3D.h"

#include "chisel/Hammer.h"
#include "chisel/Gizmos.h"
#include "chisel/VMF.h"

namespace chisel::hammer
{
    using editor::View3D;

    struct Viewport : public View3D
    {
        Viewport() : View3D(ICON_MC_IMAGE_SIZE_SELECT_ACTUAL, "Viewport", 512, 512, true) {}

        void DrawHandles(mat4x4& view, mat4x4& proj) override
        {
        }

        void OnPostDraw() override
        {
            using namespace editor;

            // TODO: Better way to find solid by ID
            Hammer.Renderer->DrawSolidsWith([&](MapEntity& ent, Solid& solid)
            {
                if (editor::Selection.Active() == Hammer.GetSelectionID(ent, solid))
                    DrawSelectionOutline(solid);
            });


            Gizmos.DrawIcon(vec3(0), Gizmos.icnLight);
        }

        void DrawSelectionOutline(Solid& solid)
        {
            editor::Tools.DrawSelectionOutline(&solid.mesh);
        }

    };
}
