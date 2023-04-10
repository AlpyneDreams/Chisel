#include "MapRender.h"

#include "console/ConVar.h"
#include "core/Transform.h"
#include "FGD/FGD.h"
#include "render/CBuffers.h"

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
        missingTexture = Assets.Load<Texture>("materials/dev_missing.png");
    }

    inline float srgb_to_linear( float fVal )
    {
        return ( fVal < 0.04045f ) ? fVal / 12.92f : std::pow( ( fVal + 0.055f) / 1.055f, 2.4f );
    }

    inline float linear_to_srgb( float fVal )
    {
        return ( fVal < 0.0031308f ) ? fVal * 12.92f : std::pow( fVal, 1.0f / 2.4f ) * 1.055f - 0.055f;
    }

    void MapRender::Update()
    {
        Camera& camera = Tools.editorCamera.camera;

        // Get camera matrices
        mat4x4 view = camera.ViewMatrix();
        mat4x4 proj = camera.ProjMatrix();

        // Update CameraState
        cbuffers::CameraState data;
        data.viewProj = proj * view;
        data.view = view;

        r.UpdateDynamicBuffer(r.cbuffers.camera.ptr(), data);
        r.ctx->VSSetConstantBuffers1(0, 1, &r.cbuffers.camera, nullptr, nullptr);

        r.ctx->ClearRenderTargetView(Tools.rt_SceneView.rtv.ptr(), Color(srgb_to_linear(0.2), srgb_to_linear(0.2), srgb_to_linear(0.2)));
        r.ctx->ClearRenderTargetView(Tools.rt_ObjectID.rtv.ptr(), Colors.Black);
        r.ctx->ClearDepthStencilView(Tools.ds_SceneView.dsv.ptr(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        ID3D11RenderTargetView* rts[] = {Tools.rt_SceneView.rtv.ptr(), Tools.rt_ObjectID.rtv.ptr()};
        r.ctx->OMSetRenderTargets(2, rts, Tools.ds_SceneView.dsv.ptr());

        float2 size = Tools.rt_SceneView.GetSize();
        D3D11_VIEWPORT viewport = { 0, 0, size.x, size.y, 0.0f, 1.0f };
        r.ctx->RSSetViewports(1, &viewport);
#if 0
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
                        cbuffers::ObjectState data;
                        data.model = glm::scale(glm::translate(mat4x4(1), point->origin), vec3(32.0f));

                        r.UpdateDynamicBuffer(r.cbuffers.object.ptr(), data);
                        r.ctx->VSSetConstantBuffers1(1, 1, &r.cbuffers.object, nullptr, nullptr);

                        //Tools.DrawSelectionOutline(&Primitives.Cube);
                    }
                    
                    if (!r_drawsprites)
                        continue;

                    r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
                    Gizmos.DrawIcon(point->origin, cls.texture ? cls.texture : Gizmos.icnObsolete);
                    r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
                }
                else
                {
                    AABB bounds = AABB(cls.bbox[0], cls.bbox[1]);
                    //r.SetTransform(glm::translate(mat4x4(1), point->origin) * bounds.ComputeMatrix());

                    //if (point->IsSelected())
                        //Tools.DrawSelectionOutline(&Primitives.Cube);

                    r.SetShader(shader);
                    //r.SetTexture(0, Tools.tex_White);
                    //r.SetUniform("u_color", color);
                   // r.DrawMesh(&Primitives.Cube);
                }
            }
            else if (r_drawsprites)
            {
                r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
                Gizmos.DrawIcon(point->origin, Gizmos.icnObsolete);
                r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
            }
        }
    }
    
    void MapRender::DrawBrushEntity(BrushEntity& ent)
    {
        static std::vector<BrushMesh*> opaqueMeshes;
        static std::vector<BrushMesh*> transMeshes;
        opaqueMeshes.clear();
        transMeshes.clear();

        r.SetShader(shader);
        for (Solid& brush : ent)
        {
            for (auto& mesh : brush.GetMeshes())
            {
                assert(mesh.alloc);

                if (mesh.material && mesh.material->translucent)
                    transMeshes.push_back(&mesh);
                else
                    opaqueMeshes.push_back(&mesh);
            }
        }

        auto DrawMesh = [&](BrushMesh* mesh)
        {
            cbuffers::BrushState data;
            data.id = mesh->brush->GetSelectionID();

            r.UpdateDynamicBuffer(r.cbuffers.brush.ptr(), data);
            r.ctx->PSSetConstantBuffers(1, 1, &r.cbuffers.brush);

            UINT stride = sizeof(VertexCSG);
            UINT vertexOffset = mesh->alloc->offset;
            UINT indexOffset = vertexOffset + mesh->vertices.size() * sizeof(VertexCSG);
            ID3D11Buffer* buffer = brushAllocator.buffer();
            ID3D11ShaderResourceView *srv = nullptr;
            bool pointSample = false;
            if (mesh->material)
            {
                if (mesh->material->baseTexture)
                    srv = mesh->material->baseTexture->srvSRGB.ptr();
            }

            if (!srv)
            {
                srv = missingTexture->srvSRGB.ptr();
                pointSample = true;
            }
            if (pointSample)
            {
                r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
            }
            r.ctx->PSSetShaderResources(0, 1, &srv);
            r.ctx->IASetVertexBuffers(0, 1, &buffer, &stride, &vertexOffset);
            r.ctx->IASetIndexBuffer(brushAllocator.buffer(), DXGI_FORMAT_R32_UINT, indexOffset);
            r.ctx->DrawIndexed(mesh->indices.size(), 0, 0);
            if (pointSample)
            {
                r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
            }
        };

        // Draw opaque meshes.
        r.SetBlendState(render::BlendFuncs::Normal);
        r.ctx->OMSetDepthStencilState(r.Depth.Default.ptr(), 0);
        for (auto* mesh : opaqueMeshes)
            DrawMesh(mesh);

        // Draw trans meshes.
        r.SetBlendState(render::BlendFuncs::Alpha);
        r.ctx->OMSetDepthStencilState(r.Depth.NoWrite.ptr(), 0);
        for (auto* mesh : transMeshes)
            DrawMesh(mesh);
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