
#include "Tools.h"
#include "common/Time.h"
#include "chisel/Gizmos.h"
#include "chisel/Selection.h"
#include "gui/ConsoleWindow.h"

#include <bit>

namespace chisel
{
    static RenderSystem& Renderer   = Tools.Renderer;
    static render::Render& r        = Tools.Render;

    void Tools::Init()
    {
        console = &systems.AddSystem<GUI::ConsoleWindow>();

        // Initialize render system
        Renderer.Start();

        // Initialize gizmos
        Gizmos.Init();

        // Load editor shaders
        sh_Color  = r.LoadShader("basic", "color");
        sh_Grid   = r.LoadShader("grid");

        // Setup editor render targets
        auto [width, height] = window->GetSize();
        rt_SceneView = r.CreateRenderTarget(width, height);
        rt_ObjectID = r.CreateRenderTarget(width, height, render::TextureFormat::R32F, render::TextureFormat::D32F);
        rt_ObjectID->SetReadBack(true);

        // Setup editor camera
        editorCamera.transform.position = vec3(2, 2.5, -3.5);
        editorCamera.transform.SetEulerAngles(vec3(30, -30, 0));
        editorCamera.camera.renderTarget = rt_SceneView;

        // Setup camera renderer
        chisel::Renderer.OnBeginFrame += [](render::Render& r) {
            chisel::Renderer.DrawCamera(chisel::Tools.editorCamera.camera, chisel::Tools.editorCamera.transform);
        };
    }

    void Tools::Loop()
    {
        systems.Start();

        Time::Seconds lastTime    = Time::GetTime();
        Time::Seconds accumulator = 0;

        while (!window->ShouldClose())
        {
            auto currentTime = Time::GetTime();
            auto deltaTime   = currentTime - lastTime;
            lastTime = currentTime;

            Time.Advance(deltaTime);

            // TODO: Whether we use deltaTime or unscaled.deltaTime affects
            // whether fixed.deltaTime needs to be changed with timeScale.
            // Perhaps the accumulator logic could go into Time.
            accumulator += Time.deltaTime;

            while (accumulator >= Time.fixed.deltaTime)
            {
                // Perform fixed updates
                systems.Tick();

                accumulator     -= Time.fixed.deltaTime;
                Time.fixed.time += Time.fixed.deltaTime;
                Time.tickCount++;
            }

            // Amount to lerp between physics steps
            [[maybe_unused]] double alpha = accumulator / Time.fixed.deltaTime;

            // Clear buffered input
            Input.Update();

            // Process input
            window->PreUpdate();

            // Perform system updates
            systems.Update();

            // Render
            Renderer.Update();

            // Present
            window->Update();

            Time.frameCount++;
        }
    }

    void Tools::Shutdown()
    {
        Renderer.Shutdown();
        delete window;
        Window::Shutdown();
    }

    void Tools::BeginSelectionPass(render::RenderContext &ctx)
    {
        ctx.SetupCamera();
        ctx.r.SetRenderTarget(chisel::Tools.rt_ObjectID);
        ctx.r.SetClearColor(true, Colors.Black);
        ctx.r.SetBlendFunc(render::BlendFuncs::Normal);
        ctx.r.SetClearDepth(true, 1.0f);
        ctx.r.SetBlendFunc(render::BlendFuncs::Normal);
        ctx.r.SetDepthTest(render::CompareFunc::LessEqual);
        ctx.r.SetDepthWrite(true);
    }

    void Tools::PickObject(uint2 mouse)
    {
        rt_ObjectID->ReadTexture([=](float* data, size_t size, size_t width)
        {
            for (float* ptr = data; ptr < data + size;)
            {
                uint x = (ptr - data) % width;
                uint y = (ptr - data) / width;
                uint id = std::bit_cast<uint>(*ptr++);

                if (mouse.x == x && mouse.y == y)
                {
                    if (id == 0)
                        Selection.Clear();
                    else
                        // subtract 1, because 0 is reserved for null
                        Selection.Select(id - 1);
                }
            }
        });
    }

    void Tools::DrawSelectionOutline(Mesh* mesh)
    {
        r.PushState();
        r.SetDepthTest(render::CompareFunc::LessEqual);
        r.SetPolygonMode(render::PolygonMode::Wireframe);
        r.SetShader(sh_Color);
        r.SetUniform("u_color", vec4(1, 0.6, 0.25, 1));
        r.DrawMesh(mesh);
        r.PopState();
    }

    void Tools::DrawSelectionOutline(Mesh* mesh, Transform& transform)
    {
        mat4x4 matrix = transform.GetTransformMatrix();
        r.SetTransform(matrix);
        DrawSelectionOutline(mesh);
    }

}