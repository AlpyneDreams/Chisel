#include "gui/Viewport.h"
#include "chisel/Gizmos.h"
#include "chisel/MapRender.h"

#include "math/Plane.h"
#include "math/Ray.h"

#include "gui/IconsMaterialCommunity.h"

namespace chisel
{
    Viewport::Viewport() : View3D(ICON_MC_IMAGE_SIZE_SELECT_ACTUAL, "Viewport", 512, 512, true) {}

    void Viewport::OnClick(uint2 mouse)
    {
        switch (activeTool)
        {
            default:
            case Tool::Select:
                Tools.PickObject(mouse);
                break;
            
            case Tool::Entity:
            case Tool::Block:
                break;
        }
    }

    void Viewport::OnResizeGrid(vec3& gridSize)
    {
        map.gridSize = gridSize;
    }

    void Viewport::DrawHandles(mat4x4& view, mat4x4& proj)
    {
        // Draw transform handles
        switch (activeTool)
        {
            case Tool::Select: default: break;

            case Tool::Translate:
            case Tool::Rotate:
            case Tool::Scale:
            case Tool::Universal:
            case Tool::Bounds:
            {
                bool snap = activeTool == Tool::Rotate ? view_rotate_snap : view_grid_snap;
                vec3 snapSize = activeTool == Tool::Rotate ? vec3(rotationSnap) : gridSize;
                Chisel.Renderer->DrawHandles(view, proj, activeTool, space, snap, snapSize);
                break;
            }

            case Tool::Block:
                // User can edit block bounds while adding blocks
                Chisel.Renderer->DrawHandles(view, proj, Tool::Bounds, space, view_grid_snap, gridSize);
                if (Handles.IsMouseOver())
                    break;
            case Tool::Entity:
            {
                Plane grid = Plane(Vectors.Zero, Vectors.Up);
                Ray ray    = GetMouseRay();
                if (float dist; grid.Intersects(ray, dist))
                {
                    vec3 point = ray.GetPoint(dist);
                    point = math::Snap(point, gridSize);
                    Handles.DrawPoint(point);

                    if (activeTool == Tool::Entity)
                    {
                        if (mouseOver && Mouse.GetButtonDown(MouseButton::Left))
                        {
                            // TODO: Entity type selection popup
                            PointEntity* pt = map.AddPointEntity("npc_personality_core");
                            pt->origin = point;
                            Selection.Clear();
                            Selection.Select(pt);
                        }
                        break;
                    }

                    if (mouseOver && Mouse.GetButtonDown(MouseButton::Left))
                    {
                        draggingBlock = true;
                        dragStartPos  = point;
                    }
                    else if (draggingBlock && Mouse.GetButtonUp(MouseButton::Left))
                    {
                        draggingBlock = false;
                        if (point != dragStartPos)
                        {
                            vec3 vec = glm::abs(point - dragStartPos);
                            vec3 center = (dragStartPos + point) / 2.f;
                            mat4x4 mtx = glm::translate(mat4x4(1), vec3(center.xy, gridSize.z * 0.5f));
                            auto& cube = map.AddCube(mtx, vec3(vec.xy, gridSize.z) * 0.5f);
                            Selection.Clear();
                            Selection.Select(&cube);
                        }
                    }

                    if (draggingBlock)
                    {
                        vec3 direction = point - dragStartPos;
                        vec3 corner1 = vec3(point.x, dragStartPos.y, point.z);
                        vec3 corner2 = vec3(dragStartPos.x, point.yz);
                        
                        Handles.DrawPoint(dragStartPos);
                        Handles.DrawPoint(corner1);
                        Handles.DrawPoint(corner2);
                        Gizmos.DrawLine(dragStartPos, corner1);
                        Gizmos.DrawLine(dragStartPos, corner2);
                        Gizmos.DrawLine(corner1, point);
                        Gizmos.DrawLine(corner2, point);
                    }
                }
                break;
            }
        }
        
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

        if (Keyboard.GetKeyUp(Key::LeftBracket) || Keyboard.GetKeyUp(Key::RightBracket))
        {
            const bool up = Keyboard.GetKeyUp(Key::RightBracket);
            if (up)
                gridSize *= 2.0f;
            else
                gridSize /= 2.0f;

            gridSize = glm::clamp(gridSize, glm::vec3(1.0f / 32.0f), glm::vec3(16384.0f));
            OnResizeGrid(gridSize);
        }
    }
}
