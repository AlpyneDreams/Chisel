#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "hammer/Hammer.h"
#include "engine/Engine.h"
#include "engine/System.h"
#include "editor/Tools.h"
#include "hammer/VMF.h"
#include "editor/Selection.h"

#include "core/Primitives.h"
#include "core/VertexLayout.h"
#include "math/Math.h"
#include <glm/gtx/normal.hpp>

namespace engine::hammer
{
    // TODO: Make this a RenderPipeline
    struct MapRender : public System
    {
    private:
        static inline VertexLayout xyz = {
            VertexAttribute::For<float>(3, VertexAttribute::Position),
            VertexAttribute::For<float>(3, VertexAttribute::Normal, true),
        };
        
    public:

        render::Render& r = Engine.Render;
        render::Shader* shader;

        void Start() final override
        {
            shader = Engine.Render.LoadShader("hammer_flat");
        }
        
        void Update() final override
        {
            r.SetRenderTarget(Tools.rt_SceneView);
            r.SetShader(shader);
            r.SetTransform(glm::identity<mat4x4>());
            
            DrawSolidsWith([&](MapEntity&, Solid& solid)
            {
                r.SetUniform("u_color", solid.editor.color);
                r.SetTransform(glm::identity<mat4x4>());
            });
        }
                
        void ForEachEntity(std::function<void(MapEntity&)> DrawFunc)
        {
            DrawFunc(Hammer.map.world);

            for (auto& ent : Hammer.map.entities)
                DrawFunc(ent);
        }

        void DrawSolidsWith(std::function<void(MapEntity&, Solid&)> DrawFunc)
        {
            ForEachEntity([&](MapEntity& entity) {
                for (auto& solid : entity.solids) {
                    DrawFunc(entity, solid);
                    r.DrawMesh(&solid.mesh);
                }
            });
        }
    };

}