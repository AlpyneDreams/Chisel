#include "chisel/Handles.h"
#include "chisel/Gizmos.h"
#include "render/CBuffers.h"
#include "glm/ext/matrix_transform.hpp"

#include <imgui.h>
#include <ImGuizmo.h>

namespace chisel
{
    inline ConVar<int> view_grid_max_radius("view_grid_max_radius", 4, "Maximum radius multiplier to extend the grid at small scales");

    struct Handles::GridVertex
    {
        vec3    pos;
        float   major;

        static constexpr D3D11_INPUT_ELEMENT_DESC Layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
    };
    static_assert(sizeof(Handles::GridVertex) == sizeof(float) * 4);

    //--------------------------------------------------
    //  ImGuizmo
    //--------------------------------------------------
    
    static ImGuizmo::OPERATION GetOperation(TransformType tool)
    {
        using enum ImGuizmo::OPERATION;
        switch (tool) {
            default:
            case TransformType::Translate: return TRANSLATE;
            case TransformType::Rotate:    return ROTATE;
            case TransformType::Scale:     return SCALE;
            case TransformType::Universal: return UNIVERSAL;
            case TransformType::Bounds:    return BOUNDS;
        }
    }

    void Handles::Begin(const Rect& viewport, bool allowAxisFlip)
    {
        ImGuizmo::BeginFrame();
        ImGuizmo::AllowAxisFlip(allowAxisFlip);
        ImGuizmo::SetRect(viewport.x, viewport.y, viewport.w, viewport.h);
        ImGuizmo::SetDrawlist();
    }

    bool Handles::Manipulate(Transform& transform, auto... args)
    {
        mat4x4 mtx;
        vec3 angles = transform.GetEulerAngles();
        ImGuizmo::RecomposeMatrixFromComponents(&transform.position[0], &angles[0], &transform.scale[0], &mtx[0][0]);

        if (Manipulate(mtx, args...))
        {
            ImGuizmo::DecomposeMatrixToComponents(&mtx[0][0], &transform.position[0], &angles[0], &transform.scale[0]);
            transform.SetEulerAngles(angles);
            return true;
        }
        return false;
    }

    bool Handles::Manipulate(mat4x4& model, const mat4x4& view, const mat4x4& proj, TransformType tool, Space space,
                    bool snap, const vec3& snapSize, const float* localBounds, const vec3& boundsSnap)
    {
        bool bounds = tool == TransformType::Bounds;

        // Only snap if we're translating or rotating
        snap = snap && (tool == TransformType::Translate || tool == TransformType::Rotate);

        // Record previous model transform
        mat4x4 prev;
        if (bounds)
            prev = model;

        // This will be updated only when not dragging the
        // bounds otherwise things get pretty funky.
        static vec3 lastBoundsSnap = vec3(0);

        bool changed = ImGuizmo::Manipulate(
            &view[0][0],
            &proj[0][0],
            GetOperation(tool),
            space == Space::World ? ImGuizmo::MODE::WORLD : ImGuizmo::MODE::LOCAL,
            &model[0][0],
            NULL, // deltaMatrix
            snap ? &snapSize[0] : NULL,
            bounds ? localBounds : NULL,
            &lastBoundsSnap[0]
        );

        if (bounds)
        {
            // Update bounds snap if not currently dragging
            if (!ImGui::IsMouseDown(0))
                lastBoundsSnap = boundsSnap;

            // Manually detect changes to the matrix
            if (!changed)
                return model != prev;
        }

        return changed;
    }

    // Transform using a matrix generated from an AABB. Automatically prevents scaling to 0.
    // Bounds are snapped on the same scale as translations. Returns a new matrix if any transformation was made.
    std::optional<mat4x4> Handles::Manipulate(const AABB& bounds, const mat4x4& view, const mat4x4& proj, TransformType tool, Space space,
                    bool snap, const vec3& snapSize)
    {
        auto mtx = bounds.ComputeMatrix();
        auto inv = glm::inverse(mtx);

        vec3 dims = bounds.Dimensions();

        // Local bounds stay the same regardless of the actual bounds
        AABB localBounds = { vec3(-0.5), vec3(0.5) };

        // Snap size has to be scaled based on the size of the selection
        vec3 boundsSnap = snapSize / dims;

        if (Manipulate(mtx, view, proj, tool, space, snap, snapSize, &localBounds.min[0], boundsSnap))
        {
            auto transform = mtx * inv;

            // If any of the new dimensions are 0, ignore the transformation.
            // This can happen when snapping with the bounds scaling tool.
            AABB newBounds = transform * bounds;
            vec3 newDims = newBounds.Dimensions();
            if ((dims.x != 0 && newDims.x == 0)
            ||  (dims.y != 0 && newDims.y == 0)
            ||  (dims.z != 0 && newDims.z == 0))
                return std::nullopt;

            return transform;
        }
        else return std::nullopt;
    }

    bool Handles::IsMouseOver() { return ImGuizmo::IsOver(); }

    void Handles::ViewManiuplate(const Rect& viewport, mat4x4& view, float length, float size, Color color)
    {
        ImGuizmo::ViewManipulate(&view[0][0], length, ImVec2(viewport.x, viewport.y), ImVec2(size, size), color.PackABGR());
    }

    void Handles::DrawGrid2(const mat4x4& view, const mat4x4& proj, float gridSize)
    {
        static mat4x4 grid = glm::identity<mat4x4>();
        ImGuizmo::DrawGrid(&view[0][0], &proj[0][0], &grid[0][0], gridSize);
    }

    void Handles::DrawTestCube(const mat4x4& view, const mat4x4& proj)
    {
        static mat4x4 cube = glm::identity<mat4x4>();
        ImGuizmo::DrawCubes(&view[0][0], &proj[0][0], &cube[0][0], 1);
    }

    //--------------------------------------------------
    //  Grid
    //--------------------------------------------------

    void Handles::DrawGrid(Camera& camera, vec3 gridSize)
    {
        auto& r = Engine.rctx;
        r.SetBlendState(render::BlendFuncs::Alpha);
        r.SetDepthStencilState(r.Depth.LessEqual.ptr());
        r.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        r.SetRasterState(r.Raster.SmoothLines.ptr());
        r.SetShader(sh_Grid);

        // Determine center of grid based on camera position
        vec3 chunk = gridSize * float(gridChunkSize);
        vec3 cameraCell = glm::floor((camera.position / chunk) + vec3(0.5));
        cameraCell.z = 0;

        mat4x4 mtx = glm::translate(mat4x4(1), cameraCell * chunk);
        mtx = glm::scale(mtx, gridSize);

        // Determine number of grid chunks to draw
        int3 radius = glm::min(int3(view_grid_max_radius), int3(glm::ceil(vec3(64.f) / gridSize)));

        // Set far Z based on radius
        vec3 farZ = glm::min(vec3(radius) * chunk * 0.8f, vec3(camera.far));

        mat4x4 view = camera.ViewMatrix();
        mat4x4 proj = camera.ProjMatrix();

        // Set camera state
        cbuffers::CameraState camState;
        camState.viewProj = proj * view;
        camState.view = view;
        camState.farZ = glm::min(farZ.x, farZ.y);
        r.UploadConstBuffer(0, r.cbuffers.camera, camState, render::VertexShader);

        // Draw each cell
        for (int x = -radius.x; x <= radius.x; x++)
        {
            for (int y = -radius.y; y <= radius.y; y++)
            {
                vec3 translation = vec3(x, y, 0) * vec3(gridChunkSize);

                cbuffers::ObjectState data;
                data.model = glm::translate(mtx, translation);
                data.id = 0;

                r.UploadConstBuffer(1, r.cbuffers.object, data, render::VertexShader);

                r.DrawMesh(grid.ptr());
            }
        }

        r.SetRasterState(r.Raster.Default.ptr());
        r.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        r.SetDepthStencilState(r.Depth.Default.ptr());
        r.SetBlendState(nullptr);
    }

    Handles::Handles()
    {
    }

    void Handles::Init()
    {
        auto& r = Engine.rctx;

        sh_Grid = render::Shader(Engine.rctx.device.ptr(), GridVertex::Layout, "grid");

        std::vector<Handles::GridVertex> gridVertices;
        grid = new Mesh();

        int radius = gridChunkSize / 2;
        int gridMajor = 8;
        gridVertices.resize((gridChunkSize + 1) * 4);

        uint i = 0;
        for (int x = -radius; x < radius; x++, i += 2)
        {
            gridVertices[i] = {vec3(x, -radius, 0), x % gridMajor == 0 ? 1.f : 0.f};
            gridVertices[i+1] = {vec3(x, +radius, 0), x % gridMajor == 0 ? 1.f : 0.f};
        }

        for (int z = -radius; z < radius; z++, i += 2)
        {
            gridVertices[i] = {vec3(-radius, z, 0), z % gridMajor == 0 ? 1.f : 0.f};
            gridVertices[i+1] = {vec3(+radius, z, 0), z % gridMajor == 0 ? 1.f : 0.f};
        }

        auto& g = grid->AddGroup();
        g.vertices.layout.Add<float>(3, VertexAttribute::Position);
        g.vertices.layout.Add<float>(1, VertexAttribute::TexCoord);
        g.vertices.pointer = &gridVertices[0];
        g.vertices.count = gridVertices.size();

        r.UploadMesh(grid.ptr());
    }
}
