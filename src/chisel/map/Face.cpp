#include "Face.h"
#include "Solid.h"

namespace chisel
{
    void Face::UpdateBounds()
    {
        // Compute the bounds from face points.
        // Does not currently account for displacement.
        if (points.size() > 0)
            bounds = AABB { points[0], points[0] };
        for (auto& point : points)
            bounds = bounds.Extend(point);

        // Fudge the bounds for ImGuizmo
        vec3 dims = bounds.Dimensions();
        if (dims.x == 0.0f)
            bounds.max.x += 0.001f;
        if (dims.y == 0.0f)
            bounds.max.y += 0.001f;
        if (dims.z == 0.0f)
            bounds.max.z += 0.001f;
    }

    void Face::Transform(const mat4x4& matrix)
    {
        // Faces will be regenerated, so deselect
        Selection.Unselect(this);

        side->plane = side->plane.Transformed(matrix);
        solid->UpdateMesh();

        // Select the new face on the same side
        for (auto& face : solid->m_faces)
        {
            if (face.side == side)
            {
                Selection.Select(&face);
                break;
            }
        }
    }
}
