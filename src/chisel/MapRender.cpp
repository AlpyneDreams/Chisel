#include "MapRender.h"

#include "console/ConVar.h"
#include "core/Transform.h"
#include "FGD/FGD.h"
#include "gui/Viewport.h"
#include "render/CBuffers.h"

namespace chisel
{
    static ConVar<bool> r_drawbrushes("r_drawbrushes", true, "Draw brushes");
    static ConVar<bool> r_drawworld("r_drawworld", true, "Draw world");
    static ConVar<bool> r_drawsprites("r_drawsprites", true, "Draw sprites");

    // Orange Tint: Color(0.8, 0.4, 0.1, 1);
    static ConVar<vec4> color_selection = ConVar<vec4>("color_selection", vec4(0.6, 0.1, 0.1, 1), "Selection color");
    static ConVar<vec4> color_selection_outline = ConVar<vec4>("color_selection_outline", vec4(0.95, 0.59, 0.19, 1), "Selection outline color");
    static ConVar<vec4> color_preview = ConVar<vec4>("color_preview", vec4(1, 1, 1, 0.5), "Placement preview color");

    MapRender::MapRender()
        : System()
    {
    }

    void MapRender::Start()
    {
        Shaders.Brush = render::Shader(r.device.ptr(), VertexSolid::Layout, "brush");
        Shaders.BrushBlend = render::Shader(r.device.ptr(), VertexSolid::Layout, "brush_blend");
        Shaders.BrushDebugID = render::Shader(r.device.ptr(), VertexSolid::Layout, "brush_debug_id");

        // Load builtin textures
        Textures.Missing = Assets.Load<Texture>("textures/error.png");
        Textures.White = Assets.Load<Texture>("textures/white.png");

        Chisel.brushAllocator = std::make_unique<BrushGPUAllocator>(r);
    }

    void MapRender::DrawViewport(Viewport& viewport)
    {
        // Get camera matrices
        Camera& camera = viewport.GetCamera();
        mat4x4 view = camera.ViewMatrix();
        mat4x4 proj = camera.ProjMatrix();

        // Update CameraState
        cbuffers::CameraState data;
        data.viewProj = proj * view;
        data.view = view;

        r.UpdateDynamicBuffer(r.cbuffers.camera.ptr(), data);
        r.ctx->VSSetConstantBuffers1(0, 1, &r.cbuffers.camera, nullptr, nullptr);

        ID3D11RenderTargetView* rts[] = {viewport.rt_SceneView->rtv.ptr(), viewport.rt_ObjectID->rtv.ptr()};
        r.ctx->OMSetRenderTargets(2, rts, viewport.ds_SceneView->dsv.ptr());

        float2 size = viewport.rt_SceneView->GetSize();
        D3D11_VIEWPORT viewrect = { 0, 0, size.x, size.y, 0.0f, 1.0f };
        r.ctx->RSSetViewports(1, &viewrect);

        r.ctx->ClearRenderTargetView(viewport.rt_SceneView->rtv.ptr(), Color(0.2, 0.2, 0.2).Linear());
        r.ctx->ClearRenderTargetView(viewport.rt_ObjectID->rtv.ptr(), Colors.Black);
        r.ctx->ClearDepthStencilView(viewport.ds_SceneView->dsv.ptr(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        drawMode = viewport.drawMode;
        if (wireframe = drawMode == Viewport::DrawMode::Wireframe)
            r.ctx->RSSetState(r.Raster.Wireframe.ptr());
        else
            r.ctx->RSSetState(r.Raster.Default.ptr());

        // TODO: Cull!
        if (r_drawbrushes)
        {
            if (r_drawworld)
                DrawBrushEntity(map);

            for (auto* entity : map.Entities())
            {
                if (BrushEntity* brush = dynamic_cast<BrushEntity*>(entity))
                    DrawBrushEntity(*brush);
            }
        }

        for (const auto* entity : map.Entities())
        {
            const PointEntity* point = dynamic_cast<const PointEntity*>(entity);
            if (!point) continue;

            DrawPointEntity(entity->classname, false, point->origin, vec3(0), point->IsSelected(), point->GetSelectionID());
        }

        r.ctx->RSSetState(r.Raster.Default.ptr());
    }

    void MapRender::DrawPointEntity(const std::string& classname, bool preview, vec3 origin, vec3 angles, bool selected, SelectionID id)
    {
        Color color = selected ? Color(color_selection) : (preview ? Color(color_preview) : Colors.White);

        if (Chisel.fgd->classes.contains(classname))
        {
            auto& cls = Chisel.fgd->classes[classname];

            AABB bounds = AABB{cls.bbox[0], cls.bbox[1]};

            if (!r_drawsprites)
                return;

            r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
            Gizmos.DrawIcon(
                origin,
                cls.texture != nullptr ? cls.texture.ptr() : Gizmos.icnObsolete.ptr(),
                color,
                id
            );
            r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
        }
        else if (r_drawsprites)
        {
            r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
            Gizmos.DrawIcon(
                origin,
                Gizmos.icnObsolete.ptr(),
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

        for (Solid& brush : ent.Brushes())
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

        auto DrawPass = [&](BrushMesh* mesh, float4 color, SelectionID id, Texture* texOverride = nullptr)
        {
            cbuffers::BrushState data;
            data.color = color;
            data.id = id;

            r.UpdateDynamicBuffer(r.cbuffers.brush.ptr(), data);
            r.ctx->PSSetConstantBuffers(1, 1, &r.cbuffers.brush);
            r.ctx->VSSetConstantBuffers(1, 1, &r.cbuffers.brush);

            UINT stride = sizeof(VertexSolid);
            UINT vertexOffset = mesh->alloc->offset;
            UINT indexOffset = vertexOffset + mesh->vertices.size() * sizeof(VertexSolid);
            ID3D11Buffer* buffer = Chisel.brushAllocator->buffer();
            ID3D11ShaderResourceView *srv = nullptr;
            bool pointSample = false;

            uint numLayers = 1;
            if (mesh->material)
            {
                // Bind $basetexture
                if (mesh->material->baseTexture != nullptr)
                    srv = mesh->material->baseTexture->srvSRGB.ptr();
                
                // Bind additional $basetexture2+ layers
                for (uint i = 0; i < std::size(mesh->material->baseTextures); i++)
                {
                    if (Texture* layer = mesh->material->baseTextures[i].ptr())
                    {
                        numLayers++;
                        r.ctx->PSSetShaderResources(i+1, 1, texOverride ? &texOverride->srvSRGB : &layer->srvSRGB);
                    }
                }
            }
            
            if (texOverride)
                srv = texOverride->srvSRGB.ptr();

            if (!srv)
            {
                srv = Textures.Missing->srvSRGB.ptr();
                pointSample = true;
            }
            if (pointSample)
            {
                r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
            }
            r.ctx->PSSetShaderResources(0, 1, &srv);

            // Choose shader variant
            if (numLayers > 1)
                r.SetShader(Shaders.BrushBlend);
            else
                r.SetShader(Shaders.Brush);

            if (this->drawMode == Viewport::DrawMode::ObjectID)
                r.SetShader(Shaders.BrushDebugID);

            r.ctx->IASetVertexBuffers(0, 1, &buffer, &stride, &vertexOffset);
            r.ctx->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, indexOffset);
            r.ctx->DrawIndexed(mesh->indices.size(), 0, 0);
            if (pointSample)
            {
                r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
            }
        };

        auto DrawMesh = [&](BrushMesh* mesh)
        {
            SelectionID id = mesh->brush->GetSelectionID();

            if (Chisel.selectMode == SelectMode::Faces)
                id = 0;

            if (mesh->brush->IsSelected())
            {
                if (wireframe)
                {
                    DrawPass(mesh, color_selection_outline, id, Textures.White.ptr());
                }
                else
                {
                    DrawPass(mesh, color_selection, id);
                    r.ctx->RSSetState(r.Raster.Wireframe.ptr());
                    DrawPass(mesh, color_selection_outline, id, Textures.White.ptr());
                    r.ctx->RSSetState(r.Raster.Default.ptr());
                }
            }
            else
            {
                DrawPass(mesh, Colors.White, id);
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
            //brush->GetBrush().AlignToGrid(view_grid_size);
        }
    }

}