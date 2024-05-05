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
        SelectionID id = 0;

        // To manage scope: Gizmo g; g.color = ...; g.DrawLine(...);
        Gizmos() {}
        void Reset();

        void DrawIcon(vec3 pos, Texture* icon, vec3 size = vec3(32.0f), const render::Shader& shader = sh_Sprite);
        void DrawPoint(vec3 pos, float scale = -1.f); // scale by default is based on grid size
        void DrawLine(vec3 start, vec3 end);
        void DrawPlane(const Plane& plane, bool backFace = true);
        void DrawBox(std::span<vec3, 8> corners);
        void DrawBox(vec3 origin, float radius = 16);
        void DrawWireBox(std::span<vec3, 8> corners);
        void DrawAABB(const AABB& aabb);
        void DrawWireAABB(const AABB& aabb);

        static void Init();
    protected:
        void PreDraw();
        void PostDraw();

        static render::RenderContext& r;
    } Gizmos;

    using Gizmo = struct Gizmos;
}