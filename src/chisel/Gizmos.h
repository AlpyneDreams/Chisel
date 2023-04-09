#pragma once

#include "chisel/Tools.h"
#include "assets/Assets.h"
#include "assets/AssetTypes.h"
#include "glm/ext/matrix_transform.hpp"
#include "render/Render.h"
#include "core/Mesh.h"
#include "core/Primitives.h"

#include <vector>
#include <glm/gtx/vector_angle.hpp>

namespace chisel
{
    inline struct Gizmos
    {
        static inline render::Texture* icnLight = nullptr;
        static inline render::Texture* icnHandle = nullptr;
#if 0
        static inline render::Shader* sh_Sprite = nullptr;
#endif

        void DrawIcon(vec3 pos, render::Texture* icon, vec3 size = vec3(64.f))
        {
#if 0
            auto& r = Tools.Render;
            r.SetShader(sh_Sprite);
            r.SetTexture(0, icon);
            r.SetBlendFunc(render::BlendFuncs::Alpha);
            r.SetDepthWrite(false);
            mat4x4 mtx = glm::scale(glm::translate(mat4x4(1.0f), pos), size);
            r.SetTransform(mtx);
            r.DrawMesh(&Primitives.Quad);
            r.SetDepthWrite(true);
#endif
        }

        // TODO: These could be batched.
        void DrawLine(vec3 start, vec3 end, Color color = Colors.White)
        {
#if 0
            auto& r = Tools.Render;
            r.SetShader(Tools.sh_Color);
            r.SetUniform("u_color", color);
            mat4x4 mtx = glm::translate(mat4x4(1), start);
            mtx = glm::rotate(mtx, glm::orientedAngle(glm::vec3(1, 0, 0), glm::normalize(end - start), Vectors.Up), glm::vec3(0, 0, 1));
            mtx = glm::scale(mtx, glm::vec3(glm::distance(start, end), 1.0f, 1.0f));
            r.SetTransform(mtx);
            r.SetPrimitiveType(render::PrimitiveType::Lines);
            r.DrawMesh(&Primitives.Line);
#endif
        }

        void Init()
        {
            icnLight = Assets.Load<TextureAsset>("textures/ui/light.png");
            icnHandle = Assets.Load<TextureAsset>("textures/ui/handle.png");
#if 0
            sh_Sprite = Tools.Render.LoadShader("billboard", "sprite");
#endif
        }
    } Gizmos;
}