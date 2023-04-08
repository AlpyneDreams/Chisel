#include "MapRender.h"

#include "console/ConVar.h"
#include "core/Transform.h"
#include "fgd/FGD.h"

namespace chisel
{
    static ConVar<bool> r_rebuildworld("r_rebuildworld", true, "Rebuild world");

    static ConVar<bool> r_drawbrushes("r_drawbrushes", true, "Draw brushes");
    static ConVar<bool> r_drawworld("r_drawworld", true, "Draw world");
    static ConVar<bool> r_drawsprites("r_drawsprites", true, "Draw sprites");

    MapRender::MapRender()
        : System()
        , brushAllocator(r)
    {
    }

    void MapRender::Start()
    {
        shader = render::Shader(r.device.ptr(), VertexCSG::Layout, "brush");
    }

    void MapRender::Update()
    {
        r.ctx->ClearRenderTargetView(Tools.rt_SceneView.rtv.ptr(), Color(0.2, 0.2, 0.2));
        r.ctx->ClearDepthStencilView(Tools.ds_SceneView.dsv.ptr(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        r.ctx->OMSetRenderTargets(1, &Tools.rt_SceneView.rtv, Tools.ds_SceneView.dsv.ptr());
#if 0
        r.SetClearDepth(true, 1.0f);
        r.SetShader(shader);
        r.SetUniform("u_color", Colors.White);
        r.SetTexture(0, Tools.tex_White);
        r.SetTransform(glm::identity<mat4x4>());
#endif

        if (r_rebuildworld)
        {
            // TODO: Refactor so we can rebuild but not upload
            // Return bool if rebuild changed to open allocator etc.
            brushAllocator.open();
            map.Rebuild(brushAllocator);
            brushAllocator.close();
        }

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

#if 0
        for (const auto* entity : map.entities)
        {
            const PointEntity* point = dynamic_cast<const PointEntity*>(entity);
            if (!point) continue;

            auto& classname = entity->classname;
            if (Chisel.fgd->classes.contains(classname))
            {
                auto& cls = Chisel.fgd->classes[classname];

                Color255 color = Color255(cls.color.r, cls.color.g, cls.color.g, 255);

                if (point->IsSelected())
                    color = Color255(255, 0, 0, 255);
                
                AABB bounds = AABB(cls.bbox[0], cls.bbox[1]);

                if (cls.texture)
                {
                    if (point->IsSelected())
                    {
                        r.SetTransform(glm::scale(glm::translate(mat4x4(1), point->origin), vec3(64.f)));
                        Tools.DrawSelectionOutline(&Primitives.Cube);
                    }
                    
                    if (!r_drawsprites)
                        continue;

                    if (cls.texture)
                        Gizmos.DrawIcon(point->origin, cls.texture);
                    else
                        Gizmos.DrawIcon(point->origin, Gizmos.icnLight);
                }
                else
                {
                    AABB bounds = AABB(cls.bbox[0], cls.bbox[1]);
                    r.SetTransform(glm::translate(mat4x4(1), point->origin) * bounds.ComputeMatrix());

                    if (point->IsSelected())
                        Tools.DrawSelectionOutline(&Primitives.Cube);

                    r.SetShader(shader);
                    r.SetTexture(0, Tools.tex_White);
                    r.SetUniform("u_color", color);
                    r.DrawMesh(&Primitives.Cube);
                }
            }
            else if (r_drawsprites)
            {
                Gizmos.DrawIcon(point->origin, Gizmos.icnLight);
            }
        }
#endif
    }
    
    void MapRender::DrawBrushEntity(BrushEntity& ent)
    {
        r.SetShader(shader);
        for (Solid& brush : ent)
        {
            for (auto& mesh : brush.GetMeshes())
            {
                //r.SetUniform("u_color", Color(1, 1, 1));//brush.GetTempColor());

#if 0
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
#endif
                assert(mesh.alloc);

                UINT stride = sizeof(VertexCSG);
                UINT vertexOffset = mesh.alloc->offset;
                UINT indexOffset = vertexOffset + mesh.vertices.size() * sizeof(VertexCSG);
                ID3D11Buffer* buffer = brushAllocator.buffer();
                r.ctx->IASetVertexBuffers(0, 1, &buffer, &stride, &vertexOffset);
                r.ctx->IASetIndexBuffer(brushAllocator.buffer(), DXGI_FORMAT_R32_UINT, indexOffset);
                r.ctx->DrawIndexed(mesh.indices.size(), 0, 0);
            }
        }
    }

    void MapRender::DrawSelectionPass()
    {
#if 0
        // TODO: Cull!
        for (Solid& brush : map)
        {
            for (auto& mesh : brush.GetMeshes())
            {
                Tools.PreDrawSelection(r, brush.GetSelectionID());
                r.DrawMesh(&mesh.mesh);
            }
        }

        for (const auto* entity : map.entities)
        {
            const PointEntity* point = dynamic_cast<const PointEntity*>(entity);
            if (!point) continue;

            auto& classname = entity->classname;
            if (Chisel.fgd->classes.contains(classname))
            {
                auto& cls = Chisel.fgd->classes[classname];

                if (cls.texture)
                {
                    mat4x4 mtx = glm::scale(glm::translate(mat4x4(1.0f), point->origin), vec3(64.f));
                    Tools.PreDrawSelection(r, point->GetSelectionID());
                    r.SetTransform(mtx);
                    r.DrawMesh(&Primitives.Quad);
                }
                else
                {
                    AABB bounds = AABB(cls.bbox[0], cls.bbox[1]);

                    Tools.PreDrawSelection(r, point->GetSelectionID());
                    r.SetTransform(glm::translate(mat4x4(1), point->origin) * bounds.ComputeMatrix());
                    r.DrawMesh(&Primitives.Cube);
                }
            }
        }
#endif
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