#pragma once

#include "chisel/Chisel.h"
#include "common/System.h"
#include "chisel/Tools.h"
#include "chisel/Selection.h"
#include "chisel/Handles.h"
#include "chisel/Gizmos.h"

#include "core/Primitives.h"
#include "common/Time.h"
#include "math/Math.h"
#include "math/Color.h"
#include <glm/gtx/normal.hpp>

namespace chisel
{
    // TODO: Make this a RenderPipeline?
    struct MapRender : public System
    {
    private:
        Map& map = Chisel.map;
    public:

        render::Render& r = Tools.Render;
        render::Shader* shader;

        void Start() final override
        {
            shader = Tools.Render.LoadShader("flat");
        }

        void Update() final override
        {
            r.SetClearColor(true, Color(0.2, 0.2, 0.2));
            r.SetClearDepth(true, 1.0f);
            r.SetRenderTarget(Tools.rt_SceneView);
            r.SetShader(shader);
            r.SetTransform(glm::identity<mat4x4>());

            map.Rebuild();

            // TODO: Cull!
            for (Solid& brush : map)
            {
                r.SetUniform("u_color", brush.GetTempColor());

                if (brush.IsSelected())
                {
                    // Draw a wire box around the brush
                    r.SetTransform(glm::identity<mat4x4>());
                    Tools.DrawSelectionOutline(&Primitives.Cube);

                    // Draw wireframe of the brush's mesh
                    r.SetTransform(glm::identity<mat4x4>());
                    Tools.DrawSelectionOutline(brush.GetMesh());

                    // Draw the actual mesh faces in red
                    r.SetUniform("u_color", Color(1, 0, 0));
                }

                r.DrawMesh(brush.GetMesh());
            }

            for (const auto& entity : map.entities)
            {
                Gizmos.DrawIcon(entity.origin, Gizmos.icnLight);
            }
        }

        void DrawSelectionPass()
        {
            // TODO: Cull!
            for (Solid& brush : map)
            {
                Tools.PreDrawSelection(r, brush.GetSelectionID());
                r.DrawMesh(brush.GetMesh());
            }
        }

        void DrawHandles(mat4x4& view, mat4x4& proj, Handles::Tool tool, Space space, bool snap, const vec3& snapSize);
    };
}
