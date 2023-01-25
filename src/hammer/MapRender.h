#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "hammer/Hammer.h"
#include "core/System.h"
#include "hammer/Tools.h"
#include "hammer/VMF.h"
#include "hammer/Selection.h"

#include "core/Primitives.h"
#include "core/VertexLayout.h"
#include "math/Math.h"
#include "math/Color.h"
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

        render::Render& r = Tools.Render;
        render::Shader* shader;

        void Start() final override
        {
            shader = Tools.Render.LoadShader("hammer_flat");
        }

        void Update() final override
        {
            r.SetClearColor(true, Color(0.2, 0.2, 0.2));
            r.SetClearDepth(true, 1.0f);
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