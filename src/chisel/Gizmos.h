#pragma once

#include "assets/Assets.h"
#include "render/Render.h"
#include "math/Math.h"
#include "Selection.h"
#include "math/Plane.h"

namespace chisel
{
    inline struct Gizmos
    {
        static inline Rc<Texture> icnObsolete;
        static inline Rc<Texture> icnHandle;
        static inline render::Shader sh_Sprite;

        void DrawIcon(vec3 pos, Texture* icon, Color color = Colors.White, SelectionID selection = 0, vec3 size = vec3(32.0f), bool depthTest = true);
        void DrawLine(vec3 start, vec3 end, Color color = Colors.White);
        void DrawPlane(const Plane& plane, Color color = Colors.White, bool backFace = true);

        void Init();
    } Gizmos;
}