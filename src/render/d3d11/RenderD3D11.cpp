#include "render/Render.h"

#include <imgui.h>
#include "gui/Common.h"
#include "gui/impl/imgui_impl_dx11.h"

namespace chisel::render
{
    void RenderContext::Init(Window* window)
    {
        D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;
        DXGI_SWAP_CHAIN_DESC swapchainDesc =
        {
            .BufferDesc =
            {
                .Width  = window->GetSize().x,
                .Height = window->GetSize().y,
                .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            },
            .SampleDesc =
            {
                .Count   = 1,
                .Quality = 0,
            },
            .BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 3,
            .OutputWindow = (HWND)window->GetNativeWindow(),
            .Windowed = TRUE,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };

        Com<ID3D11Device> localDevice;
        Com<IDXGISwapChain> localSwapchain;

        HRESULT hr;
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            &level,
            1,
            D3D11_SDK_VERSION,
            &swapchainDesc,
            &localSwapchain,
            &localDevice,
            nullptr,
            nullptr);

        if (FAILED(hr))
            return;

        localDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&device));
        localSwapchain->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&swapchain));
        device->GetImmediateContext1(&ctx);

        swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer.texture));
        device->CreateRenderTargetView(backbuffer.texture.ptr(), nullptr, &backbuffer.rtv);

        ImGui::CreateContext();

        GUI::Setup();

        ImGui_ImplDX11_Init(device.ptr(), ctx.ptr());
        ImGui_ImplDX11_NewFrame();
    }

    void RenderContext::Shutdown()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();

        backbuffer.texture = nullptr;
        backbuffer.rtv = nullptr;
        swapchain = nullptr;
        ctx = nullptr;
        device = nullptr;
    }

    void RenderContext::BeginFrame()
    {
        D3D11_TEXTURE2D_DESC desc;
        backbuffer.texture->GetDesc(&desc);

        ctx->OMSetRenderTargets(1, &backbuffer.rtv, nullptr);
        D3D11_VIEWPORT viewport =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width    = float(desc.Width),
            .Height   = float(desc.Height),
        };
        ctx->RSSetViewports(1, &viewport);

        float debugColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
        ctx->ClearRenderTargetView(backbuffer.rtv.ptr(), debugColor);
    }

    void RenderContext::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swapchain->Present(0, 0);
    }

}
