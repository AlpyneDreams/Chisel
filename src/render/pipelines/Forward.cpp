#include "Forward.h"
#include "common/Common.h"
#include "common/Filesystem.h"
#include "math/Math.h"
#include "platform/Platform.h"
#include "entity/Entity.h"
#include "entity/components/Transform.h"
#include "entity/components/MeshRenderer.h"

#include "engine/Time.h"
#include "render/RenderContext.h"

#include "core/Primitives.h"

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/string.h>

#include <cstdio>

namespace engine::render
{
    using Forward = ForwardRenderPipeline;

    static Entity teapot;

    void Forward::Init()
    {
        shader = r.LoadShader("vs_cubes", "fs_cubes");
        
        teapot.SetName("Teapot");
        teapot.AddComponent<Transform>();
        teapot.AddComponent<MeshRenderer>().mesh = &Primitives.Teapot;
    }

    void Forward::RenderFrame(RenderContext& ctx)
    {
        r.SetClearColor(true, Color(0.2, 0.2, 0.2));
        r.SetClearDepth(true, 1.0f);

        // Set shader
        r.SetShader(shader);
        
        // Set viewport and matrices
        ctx.SetupCamera();

        ctx.DrawRenderers();
    }
}