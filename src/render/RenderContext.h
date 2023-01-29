#pragma once
#include "Render.h"
#include "core/Camera.h"

#include <functional>

namespace chisel::render
{
    /**
     * High level scene rendering context
     * for use in render pipelines.
     */
    struct RenderContext
    {
        Render& r;
        Camera& camera;
        RenderContext(Render& render, Camera& camera) : r(render), camera(camera) {}

        void SetupCamera()
        {
            r.SetRenderTarget(camera.renderTarget);
            mat4x4 view = camera.ViewMatrix();
            mat4x4 proj = camera.ProjMatrix();
            r.SetViewTransform(view, proj);
        }
    };
}