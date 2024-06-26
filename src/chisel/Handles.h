#pragma once

#include "chisel/Chisel.h"
#include "core/Mesh.h"
#include "math/Math.h"
#include "math/Color.h"
#include "math/AABB.h"
#include "render/Render.h"
#include "core/Transform.h"
#include "core/Camera.h"
#include "console/ConVar.h"

#include <vector>
#include <optional>

namespace chisel
{
    inline struct Handles
    {
        struct GridVertex;
    private:
        Rc<Mesh> grid;
        render::Shader sh_Grid;

    public:

    // ImGuizmo //
        void Begin(const Rect& viewport, bool allowAxisFlip);

        bool Manipulate(Transform& transform, auto... args);
        bool Manipulate(mat4x4& model, const mat4x4& view, const mat4x4& proj, TransformType tool, Space space,
                        bool snap, const vec3& snapSize, const float* localBounds = NULL, const vec3& boundsSnap = vec3(1));
        
        // Transform using a matrix generated from an AABB. Automatically prevents scaling to 0.
        // Bounds are snapped on the same scale as translations. Returns a new matrix if any transformation was made.
        std::optional<mat4x4> Manipulate(const AABB& bounds, const mat4x4& view, const mat4x4& proj, TransformType tool, Space space,
                        bool snap, const vec3& snapSize);
        
        bool IsMouseOver();

        void ViewManiuplate(const Rect& viewport, mat4x4& view, float length = 35.f, float size = 128.f, Color color = Colors.Transparent);

        void DrawGrid2(const mat4x4& view, const mat4x4& proj, float gridSize = 100);

        void DrawTestCube(const mat4x4& view, const mat4x4& proj);

    // Grid //

        static constexpr int gridChunkSize = 200;

        void DrawGrid(Camera& camera, vec3 gridSize);

        Handles();

        void Init();

    } Handles;
}
