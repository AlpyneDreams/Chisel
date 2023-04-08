#pragma once

#include "chisel/Chisel.h"
#include "common/System.h"
#include "chisel/Tools.h"
#include "chisel/Selection.h"
#include "chisel/Handles.h"
#include "chisel/Gizmos.h"

#include "core/Primitives.h"
#include "common/Time.h"
#include "math/Math.h"
#include "math/Color.h"
#include <glm/gtx/normal.hpp>

namespace chisel
{
    // TODO: Make this a RenderPipeline?
    struct MapRender : public System
    {
    private:
        Map& map = Chisel.map;
    public:
        render::RenderContext& r = Tools.rctx;
        render::Shader shader;

        void Start() final override;
        void Update() final override;

        void DrawBrushEntity(BrushEntity& ent);

        void DrawSelectionPass();

        void DrawHandles(mat4x4& view, mat4x4& proj, Tool tool, Space space, bool snap, const vec3& snapSize);
    };
}
