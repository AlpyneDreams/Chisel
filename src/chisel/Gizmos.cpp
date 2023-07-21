#include "Gizmos.h"
#include "glm/ext/matrix_transform.hpp"
#include "chisel/Engine.h"
#include "render/CBuffers.h"
#include "core/Primitives.h"
#include <vector>
#include <glm/gtx/vector_angle.hpp>
#include "math/Winding.h"

namespace chisel
{
    void Gizmos::Init()
    {
        icnObsolete = Assets.Load<Texture>("textures/ui/obsolete.png");
        icnHandle   = Assets.Load<Texture>("textures/ui/handle.png");
        sh_Color    = render::Shader(Engine.rctx.device.ptr(), Primitives::Vertex::Layout, "color");
        sh_Sprite   = render::Shader(Engine.rctx.device.ptr(), Primitives::Vertex::Layout, "sprite");
    }

    void Gizmos::DrawIcon(vec3 pos, Texture* icon, Color color, SelectionID selection, vec3 size, bool depthTest)
    {
        auto& r = Engine.rctx;
        r.SetShader(sh_Sprite);
        r.ctx->PSSetShaderResources(0, 1, &icon->srvSRGB);
        if (selection == 0)
            r.SetBlendState(render::BlendFuncs::AlphaFirstWriteOnly);
        else
            r.SetBlendState(render::BlendFuncs::AlphaFirstOneRest);
        r.ctx->OMSetDepthStencilState(depthTest ? r.Depth.NoWrite.ptr() : r.Depth.Ignore.ptr(), 0);

        cbuffers::ObjectState data;
        data.model = glm::scale(glm::translate(mat4x4(1.0f), pos), size);
        data.color = color;
        data.id = selection;
        r.UpdateDynamicBuffer(r.cbuffers.object.ptr(), data);
        r.ctx->VSSetConstantBuffers1(1, 1, &r.cbuffers.object, nullptr, nullptr);
        r.ctx->PSSetConstantBuffers1(1, 1, &r.cbuffers.object, nullptr, nullptr);
        
        uint stride = sizeof(Primitives::Vertex);
        uint offset = 0;
        r.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        r.ctx->IASetVertexBuffers(0, 1, &Primitives.Quad, &stride, &offset);
        r.ctx->Draw(6, 0);
        r.ctx->OMSetDepthStencilState(r.Depth.Default.ptr(), 0);

        r.SetBlendState(render::BlendFuncs::Normal);
    }

    // TODO: These should be batched.
    void Gizmos::DrawLine(vec3 start, vec3 end, Color color)
    {
        auto& r = Engine.rctx;
        r.SetShader(sh_Color);

        mat4x4 mtx = glm::translate(mat4x4(1), start);
        mtx = glm::rotate(mtx, glm::orientedAngle(glm::vec3(1, 0, 0), glm::normalize(end - start), Vectors.Up), glm::vec3(0, 0, 1));
        mtx = glm::scale(mtx, glm::vec3(glm::distance(start, end), 1.0f, 1.0f));

        cbuffers::ObjectState data;
        data.model = mtx;
        data.color = color;
        data.id = 0;
        r.UpdateDynamicBuffer(r.cbuffers.object.ptr(), data);
        r.ctx->VSSetConstantBuffers1(1, 1, &r.cbuffers.object, nullptr, nullptr);
        r.ctx->PSSetConstantBuffers1(1, 1, &r.cbuffers.object, nullptr, nullptr);

        uint stride = sizeof(Primitives::Vertex);
        uint offset = 0;
        r.ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        r.ctx->IASetVertexBuffers(0, 1, &Primitives.Line, &stride, &offset);
        r.ctx->RSSetState(r.Raster.SmoothLines.ptr());
        r.ctx->Draw(2, 0);

        r.ctx->RSSetState(r.Raster.Default.ptr());
        r.ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void Gizmos::DrawPlane(const Plane& plane, Color color, bool backFace)
    {
        PlaneWinding winding;
        if (!PlaneWinding::CreateFromPlane(plane, winding))
            return;

        auto& r = Engine.rctx;
        r.SetShader(sh_Color);

        cbuffers::ObjectState data;
        data.model = glm::identity<mat4x4>();
        data.color = color;
        data.id = 0;

        Primitives::Vertex vertices[6];
        vertices[0].pos = winding.points[backFace ? 2 : 0];
        vertices[0].uv = vec2(0.0f);
        vertices[1].pos = winding.points[1];
        vertices[1].uv = vec2(0.0f);
        vertices[2].pos = winding.points[backFace ? 0 : 2];
        vertices[2].uv = vec2(0.0f);
        vertices[3].pos = winding.points[backFace ? 3 : 0];
        vertices[3].uv = vec2(0.0f);
        vertices[4].pos = winding.points[2];
        vertices[4].uv = vec2(0.0f);
        vertices[5].pos = winding.points[backFace ? 0 : 3];
        vertices[5].uv = vec2(0.0f);

        ID3D11Buffer* buffer = r.scratchVertex.ptr();

        r.ctx->OMSetDepthStencilState(r.Depth.NoWrite.ptr(), 0);
        r.UpdateDynamicBuffer(r.cbuffers.object.ptr(), data);
        r.UpdateDynamicBuffer(buffer, vertices, sizeof(Primitives::Vertex) * 6);
        r.ctx->VSSetConstantBuffers1(1, 1, &r.cbuffers.object, nullptr, nullptr);
        r.ctx->PSSetConstantBuffers1(1, 1, &r.cbuffers.object, nullptr, nullptr);
        r.SetBlendState(render::BlendFuncs::Alpha);

        uint stride = sizeof(Primitives::Vertex);
        uint offset = 0;
        r.ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        r.ctx->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
        r.ctx->Draw(6, 0);

        r.ctx->OMSetDepthStencilState(r.Depth.Default.ptr(), 0);
        r.SetBlendState(render::BlendFuncs::Normal);
    }
}