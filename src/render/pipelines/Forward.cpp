#include "Forward.h"
#include "common/Common.h"
#include "common/Filesystem.h"
#include "math/Math.h"
#include "platform/Platform.h"
#include "core/Transform.h"

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

    void Forward::Init()
    {
        shader = r.LoadShader("vs_cubes", "fs_cubes");
    }

    void Forward::RenderFrame(RenderContext& ctx)
    {
        r.SetClearColor(true, Color(0.2, 0.2, 0.2));
        r.SetClearDepth(true, 1.0f);

        // Set shader
        r.SetShader(shader);
        
        // Set viewport and matrices
        ctx.SetupCamera();

        r.DrawMesh(&Primitives.Teapot);
    }
}