#pragma once

#include "chisel/Chisel.h"
#include "common/System.h"
#include "chisel/Engine.h"
#include "chisel/Selection.h"
#include "chisel/Gizmos.h"

#include "core/Primitives.h"
#include "gui/Viewport.h"
#include "common/Time.h"
#include "math/Math.h"
#include "math/Color.h"
#include "chisel/FGD/FGD.h"

namespace chisel
{
    struct Camera;
    struct BrushPass;

    struct MapRender : public System
    {
    private:
        Map& map = Chisel.map;
    public:
        render::RenderContext& r = Engine.rctx;

        struct Shaders {
            render::Shader Brush;
            render::Shader BrushBlend;
            render::Shader BrushDebugID;
            render::Shader SpriteDebugID;
            render::Shader Model;
        } Shaders;

        struct DefaultTextures {
            Rc<Texture> Missing;
            Rc<Texture> White;
        } Textures;

        MapRender();

        void Start() final override;

        // Called by Viewport::Render
        void DrawViewport(Viewport& viewport);

        void DrawPointEntity(const std::string& classname, bool preview, vec3 origin, vec3 angles = vec3(0), bool selected = false, SelectionID id = 0, const PointEntity* ent = nullptr);
        void DrawBrushEntity(BrushEntity& ent);
        void DrawHandles(mat4x4& view, mat4x4& proj);

    protected:
        inline void DrawPass(const BrushPass& pass);
        inline void DrawSelectionOutline(BrushPass pass);
        inline void DrawMesh(BrushMesh* mesh);
        inline void DrawPixelSprite(vec3 pos, Texture* tex);
        inline void DrawObsolete(vec3 pos);

        bool wireframe = false;
        Viewport::DrawMode drawMode = Viewport::DrawMode::Shaded;
    };
}
