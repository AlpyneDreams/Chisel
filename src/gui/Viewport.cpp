#include "gui/Viewport.h"
#include "chisel/Gizmos.h"
#include "chisel/MapRender.h"

#include "gui/IconsMaterialCommunity.h"

namespace chisel
{
    Viewport::Viewport() : View3D(ICON_MC_IMAGE_SIZE_SELECT_ACTUAL, "Viewport", 512, 512, true) {}

    void Viewport::OnClick(uint2 mouse)
    {
        // Left-click: Select (or transform selection)
        Tools.PickObject(mouse);
    }

    void Viewport::OnResizeGrid(vec3& gridSize)
    {
        map.gridSize = gridSize;
    }

    void Viewport::DrawHandles(mat4x4& view, mat4x4& proj)
    {
        // Draw transform handles
        bool snap = activeTool == Tool::Rotate ? view_rotate_snap : view_grid_snap;
        vec3 snapSize = activeTool == Tool::Rotate ? vec3(rotationSnap) : gridSize;
        Chisel.Renderer->DrawHandles(view, proj, activeTool, space, snap, snapSize);
        
        // Draw view cube
        {
            const float size = 128.0f;
            Rect gizmoRect = viewport;
            gizmoRect.x = viewport.x + viewport.w - size;
            gizmoRect.w = size;
            gizmoRect.h = size;
            Handles.ViewManiuplate(gizmoRect, view, 35.f, size, Colors.Transparent);
            
            // Since we don't actually update the view matrix, just apply sped-up
            // mouselook if the user is dragging the cube with left click.
            if (IsMouseOver(gizmoRect) && Mouse.GetButtonDown(Mouse.Left))
                isDraggingCube = true;
            else if (!Mouse.GetButton(Mouse.Left))
                isDraggingCube = false;

            if (isDraggingCube && Mouse.GetButton(Mouse.Left))
                MouseLook(Mouse.GetMotion() * 5);
        }
    }

    void Viewport::OnPostDraw()
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
                    Selection.Delete();
                }
                ImGui::Separator();

                if (ImGui::Selectable( ICON_MC_GRID " Align to Grid"))
                    Selection.AlignToGrid(gridSize);

                if (ImGui::BeginMenu(ICON_MC_CUBE_OUTLINE " Change Volume"))
                {
                    if (ImGui::MenuItem(ICON_MC_CUBE "Solid"))
                        Selection.SetVolume(Volumes::Solid);

                    if (ImGui::MenuItem(ICON_MC_CUBE_OUTLINE "Air"))
                        Selection.SetVolume(Volumes::Air);

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
}
