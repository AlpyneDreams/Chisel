#include "PlacementTool.h"
#include "chisel/Chisel.h"
#include "chisel/Handles.h"
#include "chisel/Gizmos.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Viewport.h"

#include <vector>

namespace chisel
{
    extern ConVar<vec3> view_grid_size;

    struct PolygonTool final : public PlacementTool
    {
        PolygonTool() : PlacementTool("Polygon", ICON_MC_PENTAGON_OUTLINE, 103) {}

        virtual void DrawHandles(Viewport& viewport) override
        {
            PlacementTool::DrawHandles(viewport);

            // Escape: Clear points
            if (Keyboard.GetKeyDown(Key::Escape))
                m_points.clear();

            // Enter: Submit
            if (Keyboard.GetKeyDown(Key::Enter))
                CreatePolygon(m_lastNormal);


            // Draw points
            for (vec3& pt : m_points)
                Gizmos.DrawPoint(pt);

            // Draw lines
            if (m_points.size() >= 2)
            {
                for (auto i = 0; i < m_points.size() - 1; i++)
                    Gizmos.DrawLine(m_points[i], m_points[i + 1]);

#if 0
                for (size_t i = 0; i < m_points.size(); i++)
                {
                    size_t j = i == 0 ? m_points.size() - 1 : i - 1;
                    vec3 direction = glm::normalize(m_points[i] - m_points[i - 1]);
                    Plane plane = Plane(m_points[i], glm::cross(direction, m_lastNormal));

                    Gizmos.DrawPlane(plane, Color(0.0f, 1.0f, 1.0f, 0.2f));
                }
#endif 0
            }
        }

        virtual void OnMouseOver(Viewport& viewport, vec3 point, vec3 normal) override
        {
            // Draw hypothetical point
            Gizmos.DrawPoint(point);

            // Draw line from last point
            if (m_points.size() > 0)
                Gizmos.DrawLine(m_points.back(), point);
        }

        virtual void OnClick(Viewport& viewport, vec3 point, vec3 normal) override
        {
            m_lastNormal = normal;

            // If connecting back to the first point, submit
            if (m_points.size() > 1 && point == m_points[0])
                CreatePolygon(normal);
            else
                m_points.push_back(point);

            // TODO: Update preview polygon
        }

        bool ArePointsClockwise(vec3 normal) const
        {
            vec3 sum = vec3(0.0f);

            for (size_t i = 0; i < m_points.size(); i++)
            {
                size_t j = i == 0 ? m_points.size() - 1 : i - 1;
                vec3 v1 = m_points[i];
                vec3 v2 = m_points[j];

                sum += glm::cross(v1, v2);
            }
            return glm::dot(sum, normal) < 0.0f;
        }

        void CreatePolygon(vec3 normal)
        {
            if (m_points.size() < 3)
                return;

            Material* material = Chisel.activeMaterial.ptr();

            bool clockwise = ArePointsClockwise(normal);
            Console.Warn("Clockwise? {}", clockwise);

            std::vector<Side> sides;
            for (size_t i = 0; i < m_points.size(); i++)
            {
                size_t j = i == 0 ? m_points.size() - 1 : i - 1;
                vec3 direction = glm::normalize(clockwise ? m_points[i] - m_points[j] : m_points[j] - m_points[i]);
                Plane plane = Plane(m_points[i], glm::cross(direction, normal));
                sides.emplace_back(plane, material, 0.25f);
            }

            vec3 offset = m_points[0] * glm::abs(normal);

            // Cap it...
            sides.emplace_back(Plane(offset + normal * view_grid_size.value, normal), material, 0.25f);
            sides.emplace_back(Plane(offset, -normal), material, 0.25f);

            auto& brush = Chisel.map.AddBrush(sides);
            Selection.Clear();
            Selection.Select(&brush);

            // TODO: Build final brush
            m_points.clear();
        }

    private:
        std::vector<vec3> m_points;
        vec3 m_lastNormal;
    };

    static PolygonTool Instance;
}
