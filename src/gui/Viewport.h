#include "gui/View3D.h"

#include "chisel/Chisel.h"
#include "chisel/Gizmos.h"

namespace chisel
{
    using chisel::View3D;

    /**
     * The main level editor 3D view.
     */
    struct Viewport : public View3D
    {
        Viewport() : View3D(ICON_MC_IMAGE_SIZE_SELECT_ACTUAL, "Viewport", 512, 512, true) {}

        // TODO: One map per viewport
        Map& map = Chisel.map;

        void OnClick(uint2 mouse)
        {
            // Left-click: Select (or transform selection)
            Tools.PickObject(mouse);
        }

        void OnResizeGrid(vec3& gridSize)
        {
            map.gridSize = gridSize;
        }

        void DrawHandles(mat4x4& view, mat4x4& proj) override
        {
            {
                const float size = 128.0f;
                Rect gizmoViewPort = viewport;
                gizmoViewPort.x = gizmoViewPort.x + gizmoViewPort.w - size;
                mat4x4 gizmoView = view;
                Handles.ViewManiuplate(gizmoViewPort, gizmoView, 35.f, size, Colors.Transparent);
            }

            Chisel.Renderer->DrawHandles(view, proj, activeTool, space, view_grid_snap, gridSize);
        }

        void OnPostDraw() override
        {
            if (IsMouseOver(viewport))
            {
                if (Selection.Any() && (Mouse.GetButtonUp(Mouse.Right) || Mouse.GetButtonDown(Mouse.Middle)))
                {
                    ImGui::OpenPopup("Selection");
                }

                if (ImGui::BeginPopup("Selection"))
                {
                    ImGui::Text("Brush Selection");
                    ImGui::Separator();
                    ImGui::Selectable( ICON_MC_SCISSORS_CUTTING " Cut");
                    ImGui::Selectable( ICON_MC_FILE_DOCUMENT " Copy");
                    ImGui::Selectable( ICON_MC_CLIPBOARD " Paste");
                    ImGui::Separator();
                    ImGui::Selectable( ICON_MC_GRID " Align to Grid");
                    // TODO: Hook me up to the brush->AlignToGrid function
                    // when mapdoc exists
                    ImGui::EndPopup();
                }
            }

            if (Keyboard.GetKeyUp(Key::OpenBracket) || Keyboard.GetKeyUp(Key::CloseBracket))
            {
                const bool up = Keyboard.GetKeyUp(Key::CloseBracket);
                if (up)
                    gridSize *= 2.0f;
                else
                    gridSize /= 2.0f;

                gridSize = glm::clamp(gridSize, glm::vec3(1.0f / 32.0f), glm::vec3(16384.0f));
                OnResizeGrid(gridSize);
            }
            //Gizmos.DrawIcon(vec3(0), Gizmos.icnLight);
        }

    };
}
