#pragma once

#include "assets/Assets.h"
#include "render/Render.h"
#include "math/Math.h"
#include "Selection.h"
#include "math/Plane.h"

#include <span>

namespace chisel
{
    inline struct Gizmos
    {
        static inline Rc<Texture> icnObsolete;
        static inline Rc<Texture> icnHandle;
        static inline render::Shader sh_Sprite;
        static inline render::Shader sh_Color;

        Color color    = Colors.White;
        bool depthTest = true;

        // To manage scope: Gizmo g; g.color = ...; g.DrawLine(...);
        Gizmos() {}
        void Reset();

        void DrawIcon(vec3 pos, Texture* icon, SelectionID selection = 0, vec3 size = vec3(32.0f));
        void DrawPoint(vec3 pos);
        void DrawLine(vec3 start, vec3 end);
        void DrawPlane(const Plane& plane, bool backFace = true);
        void DrawBox(std::span<vec3, 8> corners);
        void DrawWireBox(std::span<vec3, 8> corners);
        void DrawAABB(const AABB& aabb);
        void DrawWireAABB(const AABB& aabb);

        static void Init();
    } Gizmos;

    using Gizmo = struct Gizmos;
}