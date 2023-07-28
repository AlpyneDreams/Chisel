#include "PlacementTool.h"
#include "chisel/Chisel.h"
#include "chisel/Gizmos.h"
#include "gui/Viewport.h"

namespace chisel
{
    static ConVar<bool> tool_placement_raycast("tool_placement_raycast", true, "Enable/disable raycast brush placement for perf debugging");

    void PlacementTool::DrawHandles(Viewport& viewport)
    {
        Map& map   = Chisel.map;
        Plane grid = Plane(Vectors.Zero, Vectors.Up);
        Ray ray    = viewport.GetMouseRay();

        bool hit = false;
        vec3 normal = vec3(0, 0, 1);
        vec3 point = vec3(0);
        
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
            float t;
            hit = ray.Intersects(grid, t);
            if (hit) {
                point = ray.GetPoint(t);

                // Set z to 0 manually to avoid precision errors.
                // This assumes the grid is at z = 0
                point.z = 0.f;
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
        }
        else if (viewport.draggingBlock && Mouse.GetButtonUp(MouseButton::Left))
        {
            viewport.draggingBlock = false;
            if (point != viewport.dragStartPos)
            {
                OnFinishDrag(viewport, point, normal);
            }
        }
    }
}
