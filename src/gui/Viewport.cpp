#include "console/Console.h"
#include "gui/Viewport.h"
#include "gui/ToolProperties.h"
#include "chisel/Gizmos.h"
#include "chisel/MapRender.h"

#include "math/Plane.h"
#include "math/Ray.h"

#include "gui/IconsMaterialCommunity.h"

namespace chisel
{
    Viewport::Viewport() : View3D(ICON_MC_IMAGE_SIZE_SELECT_ACTUAL, "Viewport", 512, 512, true) {}

    void Viewport::Start()
    {
        View3D::Start();
        OnResize(width, height); // Ballpark initial size
    }

    void Viewport::OnResize(uint width, uint height)
    {
        rt_SceneView = Tools.rctx.CreateRenderTarget(width, height);
        ds_SceneView = Tools.rctx.CreateDepthStencil(width, height);
        rt_ObjectID  = Tools.rctx.CreateRenderTarget(width, height, DXGI_FORMAT_R32_UINT);
        camera.renderTarget = &rt_SceneView;
    }

    void Viewport::Render()
    {
        Chisel.Renderer->DrawViewport(*this);
    }

    void* Viewport::GetMainTexture()
    {
        return GetTexture(drawMode).srvLinear.ptr();
    }

    void Viewport::OnClick(uint2 mouse)
    {
        switch (activeTool)
        {
            default:
            case Tool::Select:
                Tools.PickObject(mouse, rt_ObjectID);
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

                bool hit = false;
                vec3 normal = vec3(0, 0, 1);
                vec3 point = vec3(0);

#if 0
                auto r_hit = map.GetTree().QueryRay(ray);
                if (r_hit)
                {
                    hit = true;
                    normal = r_hit->face->side->plane.normal;
                    point = ray.GetPoint(r_hit->t);
                }
#endif

                if (!hit)
                {
                    float t;
                    hit = ray.Intersects(grid, t);
                    if (hit)
                        point = ray.GetPoint(t);
                }

                if (hit)
                {
                    if (view_grid_snap)
                        point = math::Snap(point, gridSize);

                    if (activeTool == Tool::Entity)
                    {
                        // Draw hypothetical entity
                        Chisel.Renderer->DrawPointEntity(Chisel.entTool.className, true, point);

                        // Place entity on click
                        if (mouseOver && Mouse.GetButtonDown(MouseButton::Left))
                        {
                            PointEntity* pt = map.AddPointEntity(Chisel.entTool.className.c_str());
                            pt->origin = point;
                            Selection.Clear();
                            Selection.Select(pt);
                        }
                        break;
                    }

                    Handles.DrawPoint(point, false);

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
                            vec3 extrude = vec3(0);
                            // If we have a degenerate axis, extrude by normal by 1 grid size.
                            if (math::CloseEnough(vec.x, 0) || math::CloseEnough(vec.y, 0) || math::CloseEnough(vec.z, 0))
                                extrude = normal * gridSize;
                            mat4x4 mtx = glm::translate(mat4x4(1), vec3(center.xyz) + (extrude * 0.5f));
                            auto& cube = map.AddCube(mtx, (vec3(vec.xyz) + extrude) * 0.5f);
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
        if (!open) {
            Tools.systems.RemoveSystem(this);
            return;
        }

        GUI::ToolPropertiesWindow(activeTool, viewport);

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
// Draw Modes //

    void Viewport::OnDrawMenuBar()
    {
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

    }

    Texture Viewport::GetTexture(Viewport::DrawMode mode)
    {
        switch (mode) {
            default: case DrawMode::Shaded:
                return rt_SceneView;
#if 0
            case DrawMode::Depth:
                return rt_SceneView->GetDepthTexture();
#endif
        }
    }
}
