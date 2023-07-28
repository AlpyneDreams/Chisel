#include "PlacementTool.h"
#include "chisel/Chisel.h"
#include "chisel/Handles.h"
#include "chisel/Gizmos.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Viewport.h"

#include <vector>

namespace chisel
{

    struct PolygonTool final : public PlacementTool
    {
        PolygonTool() : PlacementTool("Polygon", ICON_MC_PENTAGON_OUTLINE, 103) {}

        virtual void DrawHandles(Viewport& viewport) override
        {
            PlacementTool::DrawHandles(viewport);

            // Escape: Clear points
            if (Keyboard.GetKeyDown(Key::Escape))
                points.clear();

            // Enter: Submit
            if (Keyboard.GetKeyDown(Key::Enter))
                CreatePolygon();


            // Draw points
            for (vec3& pt : points)
                Gizmos.DrawPoint(pt);

            // Draw lines
            if (points.size() >= 2)
                for (auto i = 0; i < points.size() - 1; i++)
                    Gizmos.DrawLine(points[i], points[i + 1]);
        }

        virtual void OnMouseOver(Viewport& viewport, vec3 point, vec3 normal) override
        {
            // Draw hypothetical point
            Gizmos.DrawPoint(point);

            // Draw line from last point
            if (points.size() > 0)
                Gizmos.DrawLine(points.back(), point);
        }

        virtual void OnClick(Viewport& viewport, vec3 point, vec3 normal) override
        {
            // If connecting back to the first point, submit
            if (points.size() > 1 && point == points[0])
                CreatePolygon();
            else
                points.push_back(point);

            // TODO: Update preview polygon
        }

        void CreatePolygon()
        {
            if (points.size() < 3)
                return;
            
            // TODO: Build final brush
            points.clear();
        }

        std::vector<vec3> points;
    };

    static PolygonTool Instance;
}
