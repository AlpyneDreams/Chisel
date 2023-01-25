#pragma once

#include "engine/Engine.h"
#include "assets/Assets.h"
#include "glm/ext/matrix_transform.hpp"
#include "render/Render.h"
#include "render/Texture.h"
#include "core/Mesh.h"
#include "core/Primitives.h"

#include <vector>

namespace engine::editor
{
    inline struct Gizmos
    {
        static inline Texture* icnLight = nullptr;
        static inline render::Shader* sh_Sprite = nullptr;

        void DrawIcon(vec3 pos, Texture* icon)
        {
            auto& r = Engine.Render;
            r.SetShader(sh_Sprite);
            r.SetTexture(0, icnLight);
            r.SetBlendFunc(render::BlendFuncs::Alpha);
            r.SetDepthWrite(false);
            r.SetTransform(glm::translate(mat4x4(1), pos));
            r.DrawMesh(&Primitives.Quad);
            r.SetDepthWrite(true);
        }

        void Init()
        {
            icnLight = Assets.Load<Texture, ".PNG" >("textures/ui/light.png");
            sh_Sprite = Engine.Render.LoadShader("vs_billboard", "fs_sprite");
        }
    } Gizmos;
}