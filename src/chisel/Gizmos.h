#pragma once

#include "chisel/Tools.h"
#include "assets/Assets.h"
#include "glm/ext/matrix_transform.hpp"
#include "render/Render.h"
#include "render/Texture.h"
#include "core/Mesh.h"
#include "core/Primitives.h"

#include <vector>
#include <glm/gtx/vector_angle.hpp>

namespace chisel
{
    inline struct Gizmos
    {
        static inline Texture* icnLight = nullptr;
        static inline Texture* icnHandle = nullptr;
        static inline render::Shader* sh_Sprite = nullptr;

        void DrawIcon(vec3 pos, Texture* icon, vec3 size = vec3(64.f))
        {
            auto& r = Tools.Render;
            r.SetShader(sh_Sprite);
            r.SetTexture(0, icon);
            r.SetBlendFunc(render::BlendFuncs::Alpha);
            r.SetDepthWrite(false);
            //mat4x4 mtx = glm::translate(glm::scale(mat4x4(1.0f), glm::vec3(64.0f)), pos);
            mat4x4 mtx = glm::scale(glm::translate(mat4x4(1.0f), pos), size);
            r.SetTransform(mtx);
            r.DrawMesh(&Primitives.Quad);
            r.SetDepthWrite(true);
        }

        void DrawLine(vec3 start, vec3 end, Color color = Colors.White)
        {
            auto& r = Tools.Render;
            r.SetShader(Tools.sh_Color);
            r.SetUniform("u_color", color);
            mat4x4 mtx = glm::translate(mat4x4(1), start);
            mtx = glm::rotate(mtx, glm::orientedAngle(glm::vec3(1, 0, 0), glm::normalize(end - start), Vectors.Up), glm::vec3(0, 0, 1));
            mtx = glm::scale(mtx, glm::vec3(glm::distance(start, end), 1.0f, 1.0f));
            r.SetTransform(mtx);
            r.SetPrimitiveType(render::PrimitiveType::Lines);
            r.DrawMesh(&Primitives.Line);
        }

        void Init()
        {
            icnLight = Assets.Load<Texture, ".PNG" >("textures/ui/light.png");
            icnHandle = Assets.Load<Texture, ".PNG" >("textures/ui/handle.png");
            sh_Sprite = Tools.Render.LoadShader("billboard", "sprite");
        }
    } Gizmos;
}