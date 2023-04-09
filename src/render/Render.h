#pragma once

#include "assets/Asset.h"
#include "platform/Window.h"
#include "math/Math.h"
#include "math/Color.h"
#include "common/Span.h"
#include "core/Mesh.h"

#include <functional>
#include <string_view>
#include <vector>

#include "D3D11Include.h"
#include "render/BlendState.h"
#include "core/Flags.h"

namespace chisel
{
    struct Texture : Asset
    {
        Com<ID3D11Texture2D>          texture;
        Com<ID3D11ShaderResourceView> srv;

        operator bool() const { return texture != nullptr; }
        virtual uint2 GetSize()
        {
            D3D11_TEXTURE2D_DESC desc;
            texture->GetDesc(&desc);
            return uint2(desc.Width, desc.Height);
        }
    };

    struct Material : Asset
    {
        Texture* baseTexture;
        bool translucent : 1;
        bool alphatest   : 1;
    };
}

namespace chisel::render
{
    struct RenderTarget : public Texture
    {
        Com<ID3D11RenderTargetView> rtv;
    };

    struct DepthStencil : public Texture
    {
        Com<ID3D11DepthStencilView> dsv;
    };

    struct Shader
    {
        Com<ID3D11VertexShader> vs;
        Com<ID3D11PixelShader>  ps;
        Com<ID3D11InputLayout>  inputLayout;

        Shader() {}
        Shader(ID3D11Device1* device, Span<D3D11_INPUT_ELEMENT_DESC const> ia, std::string_view name);
    };

    struct ComputeShaderBuffer
    {
        Com<ID3D11Buffer> buffer;
        Com<ID3D11UnorderedAccessView> uav;
        Com<ID3D11ShaderResourceView> srv;
        Com<ID3D11Buffer> stagingBuffer = nullptr;

        void AddStagingBuffer(ID3D11Device1* device);
        void QueueDownload();
        void Download(void callback(void*));
    };

    struct ComputeShader
    {
        Com<ID3D11ComputeShader> cs;
        std::vector<ComputeShaderBuffer> buffers;

        ComputeShader() {}
        ComputeShader(ID3D11Device1* device, std::string_view name);
    };

    struct GlobalCBuffers
    {
        Com<ID3D11Buffer> camera;
        Com<ID3D11Buffer> object;
        Com<ID3D11Buffer> brush;
    };

    struct RenderContext
    {
        void Init(Window* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        RenderTarget CreateRenderTarget(uint width, uint height, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
        DepthStencil CreateDepthStencil(uint width, uint height, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);

        void CreateBlendState(const BlendState& state);
        void SetBlendState(const BlendState& state, vec4 factor = vec4(1), uint32 sampleMask = 0xFFFFFFFF);

        void SetShader(const Shader& shader);


        // Compatability with existing Mesh class
        void DrawMesh(Mesh* mesh);

        template <class T>
        ComputeShaderBuffer CreateCSOutputBuffer() { return CreateCSOutputBuffer(uint(sizeof(T))); }
        ComputeShaderBuffer CreateCSOutputBuffer(uint);

        template <class T>
        ComputeShaderBuffer CreateCSInputBuffer() { return CreateCSInputBuffer(uint(sizeof(T))); }
        ComputeShaderBuffer CreateCSInputBuffer(uint);

        template <typename T>
        Com<ID3D11Buffer> CreateCBuffer();

        template <typename T>
        void UpdateDynamicBuffer(ID3D11Resource* res, const T& data)
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            HRESULT hr = ctx->Map(res, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (FAILED(hr))
            {
                // fuck.
                return;
            }
            memcpy(mapped.pData, &data, sizeof(T));
            ctx->Unmap(res, 0);
        }

        Com<ID3D11Device1> device;
        Com<ID3D11DeviceContext1> ctx;
        Com<IDXGISwapChain> swapchain;

        RenderTarget backbuffer;

        GlobalCBuffers cbuffers;
        Com<ID3D11SamplerState> sampler;
        Com<ID3D11RasterizerState> rsState;

        struct DepthStates {
            Com<ID3D11DepthStencilState> Default = nullptr;
            Com<ID3D11DepthStencilState> NoWrite;
            Com<ID3D11DepthStencilState> LessEqual;
        } Depth;

        struct SampleStates {
            Com<ID3D11SamplerState> Default = nullptr;
            Com<ID3D11SamplerState> Point;
        } Sample;
    };
}
