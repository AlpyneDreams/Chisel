#include "MapRender.h"

#include "console/ConVar.h"
#include "core/Transform.h"
#include "FGD/FGD.h"
#include "render/CBuffers.h"

namespace chisel
{
    static ConVar<bool> r_drawbrushes("r_drawbrushes", true, "Draw brushes");
    static ConVar<bool> r_drawworld("r_drawworld", true, "Draw world");
    static ConVar<bool> r_drawsprites("r_drawsprites", true, "Draw sprites");

    // Orange Tint: Color(0.8, 0.4, 0.1, 1);
    // Selection Outline Color: Color(1, 0.6, 0.25, 1)
    static const Color SelectionColor = Color(0.6f, 0.1f, 0.1f, 1.0f);
    static const Color PreviewColor = Color(1.0f, 1.0f, 1.0f, 0.5f);

    MapRender::MapRender()
        : System()
    {
    }

    void MapRender::Start()
    {
        shader = render::Shader(r.device.ptr(), VertexSolid::Layout, "brush");
        missingTexture = Assets.Load<Texture>("textures/error.png");
        Chisel.brushAllocator = std::make_unique<BrushGPUAllocator>(r);
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

        r.ctx->ClearRenderTargetView(Tools.rt_SceneView.rtv.ptr(), Color(0.2, 0.2, 0.2).Linear());
        r.ctx->ClearRenderTargetView(Tools.rt_ObjectID.rtv.ptr(), Colors.Black);
        r.ctx->ClearDepthStencilView(Tools.ds_SceneView.dsv.ptr(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        ID3D11RenderTargetView* rts[] = {Tools.rt_SceneView.rtv.ptr(), Tools.rt_ObjectID.rtv.ptr()};
        r.ctx->OMSetRenderTargets(2, rts, Tools.ds_SceneView.dsv.ptr());

        float2 size = Tools.rt_SceneView.GetSize();
        D3D11_VIEWPORT viewport = { 0, 0, size.x, size.y, 0.0f, 1.0f };
        r.ctx->RSSetViewports(1, &viewport);

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

            DrawPointEntity(entity->classname, false, point->origin, vec3(0), point->IsSelected(), point->GetSelectionID());
        }
    }

    void MapRender::DrawPointEntity(const std::string& classname, bool preview, vec3 origin, vec3 angles, bool selected, SelectionID id)
    {
        Color color = selected ? SelectionColor : (preview ? PreviewColor : Colors.White);

        if (Chisel.fgd->classes.contains(classname))
        {
            auto& cls = Chisel.fgd->classes[classname];
            
            AABB bounds = AABB(cls.bbox[0], cls.bbox[1]);

            if (cls.texture)
            {
                if (!r_drawsprites)
                    return;

                r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
                Gizmos.DrawIcon(
                    origin,
                    cls.texture ? cls.texture : Gizmos.icnObsolete,
                    color,
                    id
                );
                r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
            }
            else
            {
                Handles.DrawPoint(origin, false);
                //AABB bounds = AABB(cls.bbox[0], cls.bbox[1]);
                //r.SetTransform(glm::translate(mat4x4(1), point->origin) * bounds.ComputeMatrix());

                //if (point->IsSelected())
                    //Tools.DrawSelectionOutline(&Primitives.Cube);

                //r.SetShader(shader);
                //r.SetTexture(0, Tools.tex_White);
                //r.SetUniform("u_color", color);
                // r.DrawMesh(&Primitives.Cube);
            }
        }
        else if (r_drawsprites)
        {
            r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
            Gizmos.DrawIcon(
                origin,
                Gizmos.icnObsolete,
                color,
                id
            );
            r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
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
            data.color = mesh->brush->IsSelected() ? SelectionColor : Colors.White;
            data.id = mesh->brush->GetSelectionID();

            r.UpdateDynamicBuffer(r.cbuffers.brush.ptr(), data);
            r.ctx->PSSetConstantBuffers(1, 1, &r.cbuffers.brush);

            UINT stride = sizeof(VertexSolid);
            UINT vertexOffset = mesh->alloc->offset;
            UINT indexOffset = vertexOffset + mesh->vertices.size() * sizeof(VertexSolid);
            ID3D11Buffer* buffer = Chisel.brushAllocator->buffer();
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
            r.ctx->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, indexOffset);
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