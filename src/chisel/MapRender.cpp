#include "MapRender.h"

#include "core/Transform.h"

namespace chisel
{
    void MapRender::DrawHandles(mat4x4& view, mat4x4& proj, Handles::Tool tool, Space space, bool snap, const vec3& snapSize)
    {
        if (Selection.Empty())
            return;

        std::optional<AABB> bounds;
        for (ISelectable* selectable : Selection)
        {
            auto selectedBounds = selectable->SelectionBounds();
            if (!selectedBounds)
                continue;

            bounds = bounds
                ? AABB::Extend(*bounds, *selectedBounds)
                : *selectedBounds;
        }

        if (!bounds)
            return;

        // TODO: seperately configured rotation snap

        auto mtx = bounds->ComputeMatrix();
        auto inv = glm::inverse(mtx);

        AABB localBounds = { vec3(-0.5), vec3(0.5) };
        vec3 boundsSnap = snapSize / bounds->Dimensions();

        if (Handles.Manipulate(mtx, view, proj, tool, space, snap, snapSize, &localBounds.min[0], boundsSnap))
        {
            auto transform = mtx * inv;

            for (ISelectable* selectable : Selection)
                selectable->SelectionTransform(transform);
            // TODO: Align to grid fights with the gizmo rn :s
            //brush->GetBrush().AlignToGrid(map.gridSize);
        }
    }

}