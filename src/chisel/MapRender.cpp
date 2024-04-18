#include "MapRender.h"

#include "console/ConVar.h"
#include "core/Transform.h"
#include "FGD/FGD.h"
#include "gui/Viewport.h"
#include "render/CBuffers.h"
#include <glm/gtx/normal.hpp>

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
        Shaders.BrushDebugID = render::Shader(r.device.ptr(), VertexSolid::Layout, "debug_id_brush");
        Shaders.SpriteDebugID = render::Shader(r.device.ptr(), Primitives::Vertex::Layout, "debug_id_sprite");

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

        r.UploadConstBuffer(0, r.cbuffers.camera, data, render::VertexShader);

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

        if (wireframe)
            r.ctx->RSSetState(r.Raster.Default.ptr());

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

        // Draw sprites
        if (Chisel.fgd->classes.contains(classname))
        {
            auto& cls = Chisel.fgd->classes[classname];

            AABB bounds = AABB{cls.bbox[0], cls.bbox[1]};

            if (!r_drawsprites)
                return;

            r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
            Gizmos.color = color;
            Gizmos.id = id;
            if (this->drawMode == Viewport::DrawMode::ObjectID)
                Gizmos.DrawIcon(origin, cls.texture != nullptr ? cls.texture.ptr() : Gizmos.icnObsolete.ptr(), vec3(32.0f), Shaders.SpriteDebugID);
            else
                Gizmos.DrawIcon(origin, cls.texture != nullptr ? cls.texture.ptr() : Gizmos.icnObsolete.ptr());
            Gizmos.id = 0;
            r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
        }
        else if (r_drawsprites)
        {
            r.ctx->PSSetSamplers(0, 1, &r.Sample.Point);
            Gizmos.color = color;
            Gizmos.id = id;
            if (this->drawMode == Viewport::DrawMode::ObjectID)
                Gizmos.DrawIcon(origin, Gizmos.icnObsolete.ptr(), vec3(32.0f), Shaders.SpriteDebugID);
            else
                Gizmos.DrawIcon(origin, Gizmos.icnObsolete.ptr());
            Gizmos.id = 0;
            r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
        }
    }

    struct BrushPass : cbuffers::BrushState
    {
        BrushMesh* mesh;
        uint indices;
        uint startIndex = 0;
        Texture* texOverride = nullptr;

        BrushPass(BrushMesh* mesh)
        {
            this->mesh = mesh;
            indices = mesh->indices.size();
            id = mesh->brush->GetSelectionID();
            color = Colors.White;
        }
    };

    inline void MapRender::DrawPass(const BrushPass& pass)
    {
        r.UploadConstBuffer(1, r.cbuffers.brush, pass);

        uint stride = sizeof(VertexSolid);
        uint vertexOffset = pass.mesh->alloc->offset;
        uint indexOffset = vertexOffset + pass.mesh->vertices.size() * stride;
        ID3D11Buffer* buffer = Chisel.brushAllocator->buffer();
        ID3D11ShaderResourceView *srv = nullptr;
        bool pointSample = false;

        uint numLayers = 1;
        if (Material* material = pass.mesh->material)
        {
            // Bind $basetexture
            if (material->baseTexture != nullptr)
                srv = material->baseTexture->srvSRGB.ptr();
            
            // Bind additional $basetexture2+ layers
            for (uint i = 0; i < std::size(material->baseTextures); i++)
            {
                if (Texture* layer = material->baseTextures[i].ptr())
                {
                    numLayers++;
                    r.ctx->PSSetShaderResources(i+1, 1, pass.texOverride ? &pass.texOverride->srvSRGB : &layer->srvSRGB);
                }
            }
        }
        
        if (pass.texOverride)
            srv = pass.texOverride->srvSRGB.ptr();

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
        r.ctx->DrawIndexed(pass.indices, pass.startIndex, 0);
        if (pointSample)
        {
            r.ctx->PSSetSamplers(0, 1, &r.Sample.Default);
        }
    }

    inline void MapRender::DrawSelectionOutline(BrushPass pass)
    {
        r.SetBlendState(render::BlendFuncs::Alpha);
        r.ctx->RSSetState(r.Raster.Wireframe.ptr());
        r.ctx->OMSetDepthStencilState(r.Depth.NoWrite.ptr(), 0);
        pass.color = color_selection_outline;
        pass.texOverride = Textures.White.ptr();
        DrawPass(pass);
        r.ctx->OMSetDepthStencilState(r.Depth.Default.ptr(), 0);
        r.ctx->RSSetState(r.Raster.Default.ptr());
        r.SetBlendState(nullptr);
    }

    inline void MapRender::DrawMesh(BrushMesh* mesh)
    {
        BrushPass pass = BrushPass(mesh);

        if (Chisel.selectMode == SelectMode::Faces)
            pass.id = 0;

        if (wireframe)
        {
            // Draw only wireframe outline
            pass.color = mesh->brush->IsSelected() ? color_selection_outline : vec4(Colors.White);
            pass.texOverride = Textures.White.ptr();
            DrawPass(pass);
        }
        else
        {
            if (mesh->brush->IsSelected())
            {
                // Highlight face
                pass.color = color_selection;
                DrawPass(pass);

                // Draw wireframe outline
                DrawSelectionOutline(pass);
            }
            else
            {
                DrawPass(pass);
            }
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

        // Draw opaque meshes.
        r.SetBlendState(wireframe ? render::BlendFuncs::Alpha : render::BlendFuncs::Normal);
        r.ctx->OMSetDepthStencilState(r.Depth.Default.ptr(), 0);
        for (auto* mesh : opaqueMeshes)
            DrawMesh(mesh);

        // Draw trans meshes.
        r.SetBlendState(render::BlendFuncs::Alpha);
        r.ctx->OMSetDepthStencilState(r.Depth.NoWrite.ptr(), 0);
        for (auto* mesh : transMeshes)
            DrawMesh(mesh);
        
        r.SetBlendState(render::BlendFuncs::Normal);
    }

    void MapRender::DrawHandles(mat4x4& view, mat4x4& proj)
    {
        if (Selection.Empty())
            return;

        if (Chisel.selectMode == SelectMode::Faces)
        {
            for (auto& item : Selection)
            {
                if (Face* face = dynamic_cast<Face*>(item); face && face->solid)
                {
                    auto& meshes = face->solid->GetMeshes();
                    if (meshes.size() > face->meshIdx)
                    {
                        auto& mesh = meshes[face->meshIdx];
                        BrushPass pass = BrushPass(&mesh);
                        pass.startIndex = face->startIndex;
                        pass.indices = face->GetDispIndexCount();
                        pass.id = face->GetSelectionID();

                        // Highlight face
                        r.ctx->RSSetState(r.Raster.DepthBiased.ptr());
                        pass.color = color_selection;
                        DrawPass(pass);

                        // Draw selection outline
                        DrawSelectionOutline(pass);
                    }
                }
            }
        }
    }

}