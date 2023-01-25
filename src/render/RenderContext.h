#pragma once
#include "Render.h"
#include "entity/Scene.h"
#include "entity/components/Camera.h"
#include "entity/components/MeshRenderer.h"

#include <functional>

namespace engine::render
{
    /** 
     * High level scene rendering context
     * for use in render pipelines.
     */
    struct RenderContext
    {
        Render& r;
        Camera& camera;
        Transform& pov;
        RenderContext(Render& render, Camera& camera, Transform& pov) : r(render), camera(camera), pov(pov) {}

        void SetupCamera()
        {
            r.SetRenderTarget(camera.renderTarget);
            mat4x4 view = camera.ViewMatrix(pov);
            mat4x4 proj = camera.ProjMatrix();
            r.SetViewTransform(view, proj);
        }

        void DrawRenderers()
        {
            for (auto&& [ent, transform, renderer] : World.Each<Transform, MeshRenderer>())
            {
                mat4x4 model = transform.GetTransformMatrix();
                r.SetTransform(model);
                r.DrawMesh(renderer.mesh);
            }
        }

        void DrawRenderersWith(std::function<void(EntityID)> func)
        {
            for (auto&& [ent, transform, renderer] : World.Each<Transform, MeshRenderer>())
            {
                mat4x4 model = transform.GetTransformMatrix();
                r.SetTransform(model);
                func(ent);
                r.DrawMesh(renderer.mesh);
            }
        }
    };
}