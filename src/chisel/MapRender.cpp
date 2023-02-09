#include "MapRender.h"

#include "console/ConVar.h"
#include "core/Transform.h"

namespace chisel
{
    static ConVar<bool> r_rebuildworld("r_rebuildworld", true, "Rebuild world");

    static ConVar<bool> r_drawbrushes("r_drawbrushes", true, "Draw brushes");
    static ConVar<bool> r_drawsprites("r_drawsprites", true, "Draw sprites");

    void MapRender::Start()
    {
        shader = Tools.Render.LoadShader("brush");
    }

    void MapRender::Update()
    {
        r.SetClearColor(true, Color(0.2, 0.2, 0.2));
        r.SetClearDepth(true, 1.0f);
        r.SetRenderTarget(Tools.rt_SceneView);
        r.SetShader(shader);
        r.SetTransform(glm::identity<mat4x4>());

        if (r_rebuildworld)
            map.Rebuild();

        // TODO: Cull!
        if (r_drawbrushes)
        {
            for (Solid& brush : map)
            {
                for (auto& mesh : brush.GetMeshes())
                {
                    r.SetUniform("u_color", Color(1, 1, 1));//brush.GetTempColor());

                    if (brush.IsSelected())
                    {
                        // Draw a wire box around the brush
                        r.SetTransform(glm::identity<mat4x4>());
                        Tools.DrawSelectionOutline(&Primitives.Cube);

                        // Draw wireframe of the brush's mesh
                        r.SetTransform(glm::identity<mat4x4>());
                        Tools.DrawSelectionOutline(&mesh.mesh);

                        // Draw the actual mesh faces in red
                        r.SetUniform("u_color", Color(1, 0, 0));
                    }

                    if (mesh.texture)
                        r.SetTexture(0, mesh.texture);

                    r.DrawMesh(&mesh.mesh);
                }
            }
        }

        if (r_drawsprites)
        {
            for (const auto* entity : map.entities)
            {
                if (const PointEntity* point = dynamic_cast<const PointEntity*>(entity))
                    Gizmos.DrawIcon(point->origin, Gizmos.icnLight);
            }
        }
    }

    void MapRender::DrawSelectionPass()
    {
        // TODO: Cull!
        for (Solid& brush : map)
        {
            for (auto& mesh : brush.GetMeshes())
            {
                Tools.PreDrawSelection(r, brush.GetSelectionID());
                r.DrawMesh(&mesh.mesh);
            }
        }
    }

    void MapRender::DrawHandles(mat4x4& view, mat4x4& proj, Handles::Tool tool, Space space, bool snap, const vec3& snapSize)
    {
        if (Selection.Empty())
            return;

        std::optional<AABB> bounds = Selection.GetBounds();

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

            Selection.Transform(transform);
            // TODO: Align to grid fights with the gizmo rn :s
            //brush->GetBrush().AlignToGrid(map.gridSize);
        }
    }

}