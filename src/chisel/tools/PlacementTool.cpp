#include "PlacementTool.h"
#include "chisel/Chisel.h"
#include "chisel/Gizmos.h"
#include "gui/Viewport.h"

namespace chisel
{
    static ConVar<bool> tool_placement_raycast("tool_placement_raycast", true, "Enable/disable raycast brush placement for perf debugging");

    void PlacementTool::DrawHandles(Viewport& viewport)
    {
        Map& map    = Chisel.map;
        Ray ray     = viewport.GetMouseRay();

        bool hit    = false;
        vec3 normal = Vectors.Up;
        vec3 point  = Vectors.Zero;
        traceMethod = 0;
        
        // Cast ray to nearest surface
        if (tool_placement_raycast)
        {
            auto r_hit = map.QueryRay(ray);
            if (r_hit)
            {
                hit = true;
                normal = r_hit->face->side->plane.normal;
                point = ray.GetPoint(r_hit->t);
            }
        }

        // If no surface hit, then cast to the grid
        if (!hit)
        {
            Plane grid = Plane(Vectors.Zero, Vectors.Up);

            float t = 0.0f;
            if (hit = ray.Intersects(grid, t))
            {
                point = ray.GetPoint(t);
                normal = grid.normal;
                traceMethod = 1;
            }
            
            // Try the local grid
            if (localGrid)
            {
                grid = Plane(gridOffset, gridNormal);

                if (float t1 = 0.0f; ray.Intersects(grid, t1))
                {
                    // Use local grid if we didn't hit the
                    // global grid, or if this hit is closer
                    if (!hit || t1 < t)
                    {
                        point = ray.GetPoint(t1);
                        normal = gridNormal;
                        hit = true;
                        traceMethod = 2;
                    }
                }
            }
        }

        if (hit)
        {
            // Snap to grid if enabled
            if (view_grid_snap)
            {
                vec3 snapped = math::Snap(point, view_grid_size);
                vec3 axis = glm::abs(normal);
                // TODO: Handle non-cardinal angles better.
                bool cardinal = math::CloseEnough(axis, vec3(1.0f, 0.0f, 0.0f)) || math::CloseEnough(axis, vec3(0.0f, 1.0f, 0.0f)) || math::CloseEnough(axis, vec3(0.0f, 0.0f, 1.0f));
                if (view_grid_snap_hit_normal || !cardinal)
                    point = snapped;
                else
                    point = (point * axis) + (snapped * (glm::vec3(1.0f) - axis));
            }

            OnMouseOver(viewport, point, normal);

            if (viewport.mouseOver && Mouse.GetButtonDown(MouseButton::Left))
            {
                OnClick(viewport, point, normal);
            }
        }
    }

    void DragTool::OnMouseOver(Viewport& viewport, vec3 point, vec3 normal)
    {
        Gizmos.DrawPoint(point, false);

        // Check for dragging
        if (viewport.mouseOver && Mouse.GetButtonDown(MouseButton::Left))
        {
            viewport.draggingBlock = true;
            viewport.dragStartPos  = point;
            SetLocalGrid(point, normal);
        }
        else if (viewport.draggingBlock && Mouse.GetButtonUp(MouseButton::Left))
        {
            viewport.draggingBlock = false;
            if (point != viewport.dragStartPos)
            {
                OnFinishDrag(viewport, point, normal);
                ClearLocalGrid();
            }
        }
    }
}
