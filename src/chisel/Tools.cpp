
#include "Tools.h"
#include "common/Time.h"
#include "chisel/Gizmos.h"
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

        // Initialize gizmos
        Gizmos.Init();

        // Load builtin textures
        tex_White = Assets.Load<Texture>("textures/white.png");

        // Load editor shaders
#if 0
        sh_Color  = r.LoadShader("basic", "color");
#endif
        sh_Grid = render::Shader(r.device.ptr(), VertexGrid::Layout, "grid");

        // Setup editor render targets
        auto [width, height] = window->GetSize();
        
        D3D11_TEXTURE2D_DESC rtDesc =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .SampleDesc =
            {
                .Count = 1,
                .Quality = 0,
            },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
        };
        r.device->CreateTexture2D(&rtDesc, nullptr, &rt_SceneView.texture);
        r.device->CreateRenderTargetView(rt_SceneView.texture.ptr(), nullptr, &rt_SceneView.rtv);
        r.device->CreateShaderResourceView(rt_SceneView.texture.ptr(), nullptr, &rt_SceneView.srv);
        float debugColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
        r.ctx->ClearRenderTargetView(rt_SceneView.rtv.ptr(), debugColor);

        D3D11_TEXTURE2D_DESC dsDesc =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D32_FLOAT,
            .SampleDesc =
            {
                .Count = 1,
                .Quality = 0,
            },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
        };
        r.device->CreateTexture2D(&dsDesc, nullptr, &ds_SceneView.texture);
        r.device->CreateDepthStencilView(ds_SceneView.texture.ptr(), nullptr, &ds_SceneView.dsv);
        r.ctx->ClearDepthStencilView(ds_SceneView.dsv.ptr(), D3D11_CLEAR_DEPTH, 1.0f, 0);
#if 0
        rt_ObjectID = r.CreateRenderTarget(width, height, render::TextureFormats::R32F, render::TextureFormats::D32F);
        rt_ObjectID->SetReadBack(true);
#endif

        // Setup editor camera
        editorCamera.camera.position = vec3(-64.0f, -32.0f, 32.0f) * 32.0f;
        editorCamera.camera.angles = math::radians(vec3(-30.0f, 30.0f, 0));
        editorCamera.camera.renderTarget = rt_SceneView;

        // Setup camera renderer
#if 0
        chisel::Renderer.OnBeginFrame += [](render::Render& r) {
            chisel::Renderer.DrawCamera(chisel::Tools.editorCamera.camera);
        };
#endif
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
#if 0
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
                    {
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
                }
            }
        });
#endif
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
