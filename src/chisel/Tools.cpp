
#include "Tools.h"
#include "common/Time.h"
#include "chisel/Gizmos.h"
#include "chisel/Handles.h"
#include "chisel/Selection.h"
#include "gui/ConsoleWindow.h"
#include "assets/Assets.h"

#include <bit>

namespace chisel
{
#if 0
    extern ConVar<render::MSAA> r_msaa;
#endif

    static RenderSystem& Renderer   = Tools.Renderer;
    static render::RenderContext& r = Tools.Renderer.rctx;

    void Tools::Init()
    {
        console = &systems.AddSystem<GUI::ConsoleWindow>();

        // Initialize render system
        Renderer.Start();

        // Setup gizmos and handles
        Primitives.Init();
        Gizmos.Init();
        Handles.Init();

        // Load builtin textures
        tex_White = Assets.Load<Texture>("textures/white.png");

        // Load editor shaders
#if 0
        sh_Color  = r.LoadShader("basic", "color");
#endif
        cs_ObjectID = render::ComputeShader(r.device.ptr(), "objectid");
        cs_ObjectID.buffers.push_back(r.CreateCSInputBuffer<uint2>());
        cs_ObjectID.buffers.push_back(r.CreateCSOutputBuffer<uint>());
        cs_ObjectID.buffers[1].AddStagingBuffer(r.device.ptr());

        // Setup editor render targets
        auto [width, height] = window->GetSize();

        float debugColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

        rt_SceneView = r.CreateRenderTarget(width, height);
        r.ctx->ClearRenderTargetView(rt_SceneView.rtv.ptr(), debugColor);

        ds_SceneView = r.CreateDepthStencil(width, height);
        r.ctx->ClearDepthStencilView(ds_SceneView.dsv.ptr(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        rt_ObjectID = r.CreateRenderTarget(width, height, DXGI_FORMAT_R32_UINT);

        // Setup editor camera
        editorCamera.camera.position = vec3(-64.0f, -32.0f, 32.0f) * 32.0f;
        editorCamera.camera.angles = math::radians(vec3(-30.0f, 30.0f, 0));
        editorCamera.camera.renderTarget = &rt_SceneView;
    }

    void Tools::ResizeViewport(uint width, uint height)
    {
        rt_SceneView = r.CreateRenderTarget(width, height);
        ds_SceneView = r.CreateDepthStencil(width, height);
        rt_ObjectID = r.CreateRenderTarget(width, height, DXGI_FORMAT_R32_UINT);
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

    void Tools::BeginSelectionPass(render::RenderContext &rctx)
    {
    }

    void Tools::PickObject(uint2 mouse)
    {
        auto bufferIn = cs_ObjectID.buffers[0];

        // Update input buffer
        r.UpdateDynamicBuffer(bufferIn.buffer.ptr(), mouse);

        // When rendering completes...
        Renderer.OnEndFrame.Once([](render::RenderContext& r)
        {
            // Unbind render targets
            r.ctx->OMSetRenderTargets(0, nullptr, nullptr);

            extern class Tools Tools;
            auto bufferIn = Tools.cs_ObjectID.buffers[0];
            auto bufferOut = Tools.cs_ObjectID.buffers[1];

            // Bind the render target, input coords, output value
            ID3D11ShaderResourceView* srvs[] = { Tools.rt_ObjectID.srvLinear.ptr(), bufferIn.srv.ptr() };
            r.ctx->CSSetShaderResources(0, 2, srvs);
            r.ctx->CSSetUnorderedAccessViews(0, 1, &bufferOut.uav, nullptr);

            // Run the compute shader
            r.ctx->CSSetShader(Tools.cs_ObjectID.cs.ptr(), nullptr, 0);
            r.ctx->Dispatch(1, 1, 1);

            // Download the output value
            bufferOut.Download([](void* data)
            {
                uint id = ((uint*)data)[0];

                if (id == 0) {
                    Selection.Clear();
                } else {
                    Selectable* selection = Selection.Find(id);
                    if (selection)
                    {
                        if (Keyboard.ctrl)
                        {
                            Selection.Toggle(selection);
                        }
                        else
                        {
                            Selection.Clear();
                            Selection.Select(selection);
                        }
                    }
                }
            });
        });
    }

#if 0
    ConVar<render::MSAA> r_msaa("r_msaa", render::MSAA::x4, "Set MSAA level", [](render::MSAA& value)
    {
        if (!IsValid(value))
            value = render::MSAA::None;
        
        if (Tools.rt_SceneView)
            Tools.rt_SceneView->SetMSAA(value);
    });
#endif
}
