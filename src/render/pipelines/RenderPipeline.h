#pragma once

#include "render/Render.h"
#include "render/RenderContext.h"

namespace engine::render
{
    class RenderPipeline
    {
    protected:
        Render& r;
    public:
        RenderPipeline(Render& render) : r(render) {}

        virtual ~RenderPipeline() {}
        virtual void Init() = 0;
        virtual void RenderFrame(RenderContext& ctx) = 0;
    };
}