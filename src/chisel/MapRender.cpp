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
        Shaders.Brush = render::Shader(r.device.ptr(), VertexSolid::InputLayout, "brush");
        Shaders.BrushBlend = render::Shader(r.device.ptr(), VertexSolid::InputLayout, "brush_blend");
        Shaders.BrushDebugID = render::Shader(r.device.ptr(), VertexSolid::InputLayout, "debug_id_brush");
        Shaders.SpriteDebugID = render::Shader(r.device.ptr(), Primitives::Vertex::Layout, "debug_id_sprite");
        Shaders.Model = render::Shader(r.device.ptr(), VertexSolid::InputLayout, "model");

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
            r.SetRasterState(r.Raster.Wireframe.ptr());
        else
            r.SetRasterState(r.Raster.Default.ptr());

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
            r.SetRasterState(r.Raster.Default.ptr());

        for (const auto* entity : map.Entities())
        {
            const PointEntity* point = dynamic_cast<const PointEntity*>(entity);
            if (!point) continue;

            DrawPointEntity(entity->classname, false, point->origin, vec3(0), point->IsSelected(), point->GetSelectionID(), point);
        }

        r.SetRasterState(r.Raster.Default.ptr());
    }

    void MapRender::DrawPointEntity(const std::string& classname, bool preview, vec3 origin, vec3 angles, bool selected, SelectionID id, const PointEntity* ent)
    {
        const Color color = selected ? Color(color_selection) : (preview ? Color(color_preview) : Colors.White);

        Gizmos.color = color;
        Gizmos.id = id;

        // TODO: Should cache FGD class with each ent...
        if (!Chisel.fgd->classes.contains(classname))
        {
            DrawObsolete(origin);
            Gizmos.id = 0;
            return;
        }

        bool drew = false;

        auto& cls = Chisel.fgd->classes[classname];

        //AABB bounds = AABB{cls.bbox[0], cls.bbox[1]};
        // TODO: Draw boxes if no sprite

        // Draw models
        if (cls.model != nullptr || cls.isProp)
        {
            Rc<Mesh> model = cls.isProp ? ent->GetModel() : cls.model;
            if (model != nullptr)
            {
                // Just upload it if it's not uploaded
                if (!model->uploaded) [[unlikely]]
                    r.UploadMesh(model.ptr());

                r.SetShader(Shaders.Model);
                r.ctx->PSSetShaderResources(0, 1, &Textures.White->srvSRGB);

                cbuffers::ObjectState data;
                data.color = color;
                data.id = id;
                data.model = glm::translate(glm::identity<mat4x4>(), origin);

                r.UploadConstBuffer(1, r.cbuffers.object, data);

                r.SetDepthStencilState(r.Depth.Default);
                r.SetRasterState(r.Raster.Default);
                r.SetSampler(0, r.Sample.Default);

                r.DrawMesh(model.ptr());
                drew = true;
            }
        }

        // Draw sprites
        if (r_drawsprites && cls.texture != nullptr)
        {
            DrawPixelSprite(origin, cls.texture.ptr());
            drew = true;
        }

        if (!drew)
            DrawObsolete(origin);

        Gizmos.color = Colors.White;
        Gizmos.id = 0;
    }

    inline void MapRender::DrawPixelSprite(vec3 pos, Texture* tex)
    {
        r.SetSampler(0, r.Sample.Default);
        if (this->drawMode == Viewport::DrawMode::ObjectID)
            Gizmos.DrawIcon(pos, tex != nullptr ? tex : Gizmos.icnObsolete.ptr(), vec3(32.0f), Shaders.SpriteDebugID);
        else
            Gizmos.DrawIcon(pos, tex != nullptr ? tex : Gizmos.icnObsolete.ptr(), vec3(32.0f));
        r.SetSampler(0, r.Sample.Point);

    }
    
    inline void MapRender::DrawObsolete(vec3 pos)
    {
        if (r_drawsprites)
            DrawPixelSprite(pos, nullptr);
        else
            Gizmos.DrawPoint(pos);
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

        // TODO: Consistent material binding mechanism for all materials
        // e.g. r.Bind(material)

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
        r.SetRasterState(r.Raster.Wireframe.ptr());
        r.SetDepthStencilState(r.Depth.NoWrite.ptr());
        pass.color = color_selection_outline;
        pass.texOverride = Textures.White.ptr();
        DrawPass(pass);
        r.SetDepthStencilState(r.Depth.Default.ptr());
        r.SetRasterState(r.Raster.Default.ptr());
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
        r.SetDepthStencilState(r.Depth.Default.ptr());
        for (auto* mesh : opaqueMeshes)
            DrawMesh(mesh);

        // Draw trans meshes.
        r.SetBlendState(render::BlendFuncs::Alpha);
        r.SetDepthStencilState(r.Depth.NoWrite.ptr());
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
                        r.SetRasterState(r.Raster.DepthBiased.ptr());
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