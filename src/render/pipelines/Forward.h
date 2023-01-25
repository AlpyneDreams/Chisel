#pragma once

#include "RenderPipeline.h"
#include "core/Mesh.h"

namespace engine::render
{
    class ForwardRenderPipeline final : public RenderPipeline
    {
        Shader* shader;

    public:
        using RenderPipeline::RenderPipeline;

        void Init() final override;
        void RenderFrame(RenderContext& ctx) final override;
    };
}