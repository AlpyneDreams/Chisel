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

        bool isDraggingCube = false;

        void DrawHandles(mat4x4& view, mat4x4& proj) override
        {
            // Draw transform handles
            Chisel.Renderer->DrawHandles(view, proj, activeTool, space, view_grid_snap, gridSize);
            
            // Draw view cube
            {
                const float size = 128.0f;
                Rect gizmoRect = viewport;
                gizmoRect.x = viewport.x + viewport.w - size;
                Handles.ViewManiuplate(gizmoRect, view, 35.f, size, Colors.Transparent);
                
                // Since we don't actually update the view matrix, just apply sped-up
                // mouselook if the user is dragging the cube with left click.
                if ((isDraggingCube || IsMouseOver(gizmoRect)) && Mouse.GetButton(Mouse.Left))
                {
                    isDraggingCube = true;
                    MouseLook(Mouse.GetMotion() * 5);
                }
                else
                {
                    isDraggingCube = false;
                }
            }
        }

        void OnPostDraw() override
        {
            if (IsMouseOver(viewport))
            {
                if (!Selection.Empty() && (/*Mouse.GetButtonUp(Mouse.Right) ||*/ Mouse.GetButtonDown(Mouse.Middle)))
                {
                    ImGui::OpenPopup("Selection");
                }

                if (ImGui::BeginPopup("Selection"))
                {
                    ImGui::Text("Brush Selection");
                    ImGui::Separator();
                    ImGui::Selectable( ICON_MC_SCISSORS_CUTTING " Cut (TODO)");
                    ImGui::Selectable( ICON_MC_FILE_DOCUMENT " Copy (TODO)");
                    ImGui::Selectable( ICON_MC_CLIPBOARD " Paste (TODO)");
                    if (ImGui::Selectable( ICON_MC_TRASH_CAN " Delete"))
                    {
                        for (ISelectable* selectable : Selection)
                            selectable->SelectionDelete();
                    }
                    ImGui::Separator();
                    if (ImGui::Selectable( ICON_MC_GRID " Align to Grid"))
                    {
                        for (ISelectable* selectable : Selection)
                            selectable->SelectionAlignToGrid(gridSize);
                    }
                    if (ImGui::BeginMenu(ICON_MC_CUBE_OUTLINE " Change Volume"))
                    {
                        if (ImGui::MenuItem(ICON_MC_CUBE "Solid"))
                        {
                            for (ISelectable* selectable : Selection)
                                selectable->SelectionSetVolume(Volumes::Solid);
                        }
                        if (ImGui::MenuItem(ICON_MC_CUBE_OUTLINE "Air"))
                        {
                            for (ISelectable* selectable : Selection)
                                selectable->SelectionSetVolume(Volumes::Air);
                        }
                        ImGui::EndMenu();
                    }
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
