#include "MapRender.h"

#include "console/ConVar.h"
#include "core/Transform.h"

namespace chisel
{
    static ConVar<bool> r_rebuildworld("r_rebuildworld", true, "Rebuild world");

    static ConVar<bool> r_drawbrushes("r_drawbrushes", true, "Draw brushes");
    static ConVar<bool> r_drawworld("r_drawworld", true, "Draw world");
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
            if (r_drawworld)
                DrawBrushEntity(map);

            for (auto* entity : map.entities)
            {
                if (BrushEntity* brush = dynamic_cast<BrushEntity*>(entity))
                    DrawBrushEntity(*brush);
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
    
    void MapRender::DrawBrushEntity(BrushEntity& ent)
    {
        for (Solid& brush : ent)
        {
            for (auto& mesh : brush.GetMeshes())
            {
                r.SetUniform("u_color", Color(1, 1, 1));//brush.GetTempColor());

                if (brush.IsSelected())
                {
                    // Draw a wire box around the brush
                    //r.SetTransform(brush.GetBounds()->ComputeMatrix());
                    //Tools.DrawSelectionOutline(&Primitives.Cube);

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

    void MapRender::DrawHandles(mat4x4& view, mat4x4& proj, Tool tool, Space space, bool snap, const vec3& snapSize)
    {
        if (Selection.Empty())
            return;

        std::optional<AABB> bounds = Selection.GetBounds();

        if (!bounds)
            return;

        if (auto transform = Handles.Manipulate(bounds.value(), view, proj, tool, space, snap, snapSize))
        {
            Selection.Transform(transform.value());
            // TODO: Align to grid fights with the gizmo rn :s
            //brush->GetBrush().AlignToGrid(map.gridSize);
        }
    }

}