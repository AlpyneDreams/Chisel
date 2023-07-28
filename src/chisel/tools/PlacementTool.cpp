#include "PlacementTool.h"
#include "chisel/Chisel.h"
#include "chisel/Gizmos.h"
#include "gui/Viewport.h"

namespace chisel
{
    static ConVar<bool> r_raycast_brush_placement("r_raycast_brush_placement", true, "Enable/disable raycast brush placement for perf debugging");

    void PlacementTool::DrawHandles(Viewport& viewport)
    {
        Map& map   = Chisel.map;
        Plane grid = Plane(Vectors.Zero, Vectors.Up);
        Ray ray    = viewport.GetMouseRay();

        bool hit = false;
        normal = vec3(0, 0, 1);
        point = vec3(0);
        plane = Plane();
        
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
            if (hit) {
                point = ray.GetPoint(t);

                // Set z to 0 manually to avoid precision errors.
                // This assumes the grid is at z = 0
                point.z = 0.f;
            }
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

            OnRayHit(viewport);
        }
    }

    void DragTool::OnRayHit(Viewport& viewport)
    {
        Gizmos.DrawPoint(point, false);

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
                OnFinishDrag(viewport);
            }
        }
    }
}
