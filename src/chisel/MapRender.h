#pragma once

#include "chisel/Chisel.h"
#include "common/System.h"
#include "chisel/Tools.h"
#include "chisel/Selection.h"
#include "chisel/Handles.h"
#include "chisel/Gizmos.h"

#include "core/Primitives.h"
#include "gui/Viewport.h"
#include "common/Time.h"
#include "math/Math.h"
#include "math/Color.h"
#include "chisel/FGD/FGD.h"
#include <glm/gtx/normal.hpp>

namespace chisel
{
    struct Camera;

    struct MapRender : public System
    {
    private:
        Map& map = Chisel.map;
    public:
        render::RenderContext& r = Tools.rctx;

        struct Shaders {
            render::Shader Brush;
            render::Shader BrushBlend;
        } Shaders;

        struct DefaultTextures {
            Rc<Texture> Missing;
            Rc<Texture> White;
        } Textures;

        MapRender();

        void Start() final override;

        // Called by Viewport::Render
        void DrawViewport(Viewport& viewport);

        void DrawPointEntity(const std::string& classname, bool preview, vec3 origin, vec3 angles = vec3(0), bool selected = false, SelectionID id = 0);
        void DrawBrushEntity(BrushEntity& ent);
        void DrawHandles(mat4x4& view, mat4x4& proj, Tool tool, Space space, bool snap, const vec3& snapSize);

        bool wireframe = false;
    };
}
