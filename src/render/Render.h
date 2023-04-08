#pragma once

#include "platform/Window.h"
#include "math/Math.h"
#include "math/Color.h"
#include "render/TextureFormat.h"
#include "render/BlendMode.h"

#include <functional>
#include <string_view>

#include "D3D11Include.h"

namespace chisel::render
{
    struct Texture
    {
        Com<ID3D11Texture2D>          texture;
        Com<ID3D11ShaderResourceView> srv;
    };

    struct RenderTarget : public Texture
    {
        Com<ID3D11RenderTargetView>   rtv;
    };

    struct RenderContext
    {
        void Init(Window* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        Com<ID3D11Device1> device;
        Com<ID3D11DeviceContext1> ctx;
        Com<IDXGISwapChain> swapchain;

        RenderTarget backbuffer;
    };
}
