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

    void Viewport::OnResizeGrid(const vec3& gridSize)
    {
        map.gridSize = gridSize;
    }

    ConVar<bool> r_raycast_brush_placement("r_raycast_brush_placement", true, "Enable/disable raycast brush placement for perf debugging");

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
                vec3 snapSize = activeTool == Tool::Rotate ? vec3(view_rotate_snap_angle) : view_grid_size;
                Chisel.Renderer->DrawHandles(view, proj, activeTool, Chisel.transformSpace, snap, snapSize);
                break;
            }

            case Tool::Block:
                // User can edit block bounds while adding blocks
                Chisel.Renderer->DrawHandles(view, proj, Tool::Bounds, Chisel.transformSpace, view_grid_snap, view_grid_size);
                if (Handles.IsMouseOver())
                    break;
            case Tool::Clip:
            case Tool::Entity:
            {
                Plane grid = Plane(Vectors.Zero, Vectors.Up);
                Ray ray    = GetMouseRay();

                bool hit = false;
                vec3 normal = vec3(0, 0, 1);
                vec3 point = vec3(0);
                Plane plane = Plane();
                
                if (r_raycast_brush_placement)
                {
                    auto r_hit = map.QueryRay(ray);
                    if (r_hit)
                    {
                        hit = true;
                        normal = r_hit->face->side->plane.normal;
                        point = ray.GetPoint(r_hit->t);
                    }
                }

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
                    {
                        vec3 snapped = math::Snap(point, view_grid_size);
                        vec3 axis = glm::abs(normal);
                        // TODO: Handle non-cardinal angles better.
                        bool cardinal = math::CloseEnough(axis, vec3(1.0f, 0.0f, 0.0f)) || math::CloseEnough(axis, vec3(0.0f, 1.0f, 0.0f)) || math::CloseEnough(axis, vec3(0.0f, 0.0f, 1.0f));
                        if (view_grid_snap_hit_normal || !cardinal)
                        {
                            point = snapped;
                        }
                        else
                        {
                            point = (point * axis) + (snapped * (glm::vec3(1.0f) - axis));
                        }
                    }

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

                    if (activeTool == Tool::Clip && draggingBlock)
                    {
                        vec3 direction = glm::normalize(point - dragStartPos);

                        plane = Plane(point, glm::cross(direction, normal));
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
                            if (activeTool == Tool::Clip)
                            {

                            }
                            else
                            {
                                vec3 vec = glm::abs(point - dragStartPos);
                                vec3 center = (dragStartPos + point) / 2.f;
                                vec3 extrude = vec3(0);
                                // If we have a degenerate axis, extrude by normal by 1 grid size.
                                if (math::CloseEnough(vec.x, 0) || math::CloseEnough(vec.y, 0) || math::CloseEnough(vec.z, 0))
                                    extrude = normal * view_grid_size.value;
                                mat4x4 mtx = glm::translate(mat4x4(1), vec3(center.xyz) + (extrude * 0.5f));
                                vec3 size = (vec3(vec.xyz) + extrude) * 0.5f;
                                // make sure we are not degenerate size.
                                bool degenerate = math::CloseEnough(size.x, 0.0f) || math::CloseEnough(size.y, 0.0f) || math::CloseEnough(size.z, 0.0f);
                                if (!degenerate)
                                {
                                    auto& cube = map.AddCube(Chisel.activeMaterial, mtx, size);
                                    Selection.Clear();
                                    Selection.Select(&cube);
                                }
                            }
                        }
                    }

                    if (draggingBlock)
                    {
                        if (activeTool == Tool::Clip)
                        {
                            Handles.DrawPoint(dragStartPos);
                            Gizmos.DrawPlane(plane, Color(0.0f, 1.0f, 1.0f, 0.1f));
                        }
                        else
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

        GUI::ToolPropertiesWindow(activeTool, viewport, instance);

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
                    Selection.AlignToGrid(view_grid_size);

                ImGui::EndPopup();
            }
        }

        if (Keyboard.GetKeyUp(Key::LeftBracket) || Keyboard.GetKeyUp(Key::RightBracket))
        {
            const bool up = Keyboard.GetKeyUp(Key::RightBracket);
            if (up)
                view_grid_size.value *= 2.0f;
            else
                view_grid_size.value /= 2.0f;

            view_grid_size = glm::clamp(view_grid_size.value, glm::vec3(1.0f / 32.0f), glm::vec3(16384.0f));
            OnResizeGrid(view_grid_size);
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
            default:
            case DrawMode::Shaded:
            case DrawMode::Wireframe:
                return rt_SceneView;
#if 0
            case DrawMode::Depth:
                return rt_SceneView->GetDepthTexture();
#endif
        }
    }
}
