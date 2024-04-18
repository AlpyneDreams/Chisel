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

namespace chisel
{
    struct Texture : Asset
    {
        using Asset::Asset;

        Com<ID3D11Texture2D>          texture;
        Com<ID3D11ShaderResourceView> srvLinear;
        Com<ID3D11ShaderResourceView> srvSRGB;

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
        Material(const fs::Path& path)
            : Asset(path)
        {
            translucent = 0;
            alphatest = 0;
        }

        Rc<Texture> baseTexture;
        Rc<Texture> baseTextures[3]; // Additional layers
        bool translucent : 1;
        bool alphatest   : 1;
    };
}

namespace chisel::render
{
    struct RenderTarget : public Texture
    {
        using Texture::Texture;

        Com<ID3D11RenderTargetView> rtv;
    };

    struct DepthStencil : public Texture
    {
        using Texture::Texture;

        Com<ID3D11DepthStencilView> dsv;
    };

    enum ShaderStages
    {
        VertexShader = 1,
        PixelShader  = 2,
        // etc.
    };

    inline ShaderStages operator|(ShaderStages a, ShaderStages b) { return ShaderStages(int(a) | int(b)); }
    inline ShaderStages operator&(ShaderStages a, ShaderStages b) { return ShaderStages(int(a) & int(b)); }

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

        Rc<RenderTarget> CreateRenderTarget(uint width, uint height, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8X8_UNORM);
        Rc<DepthStencil> CreateDepthStencil(uint width, uint height, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);

        void CreateBlendState(const BlendState& state);
        void SetBlendState(const BlendState& state, vec4 factor = vec4(1), uint32 sampleMask = 0xFFFFFFFF);

        void SetShader(const Shader& shader);


        // Compatability with existing Mesh class
        void DrawMesh(Mesh* mesh);
        void UploadMesh(Mesh* mesh);

        template <class T>
        ComputeShaderBuffer CreateCSOutputBuffer() { return CreateCSOutputBuffer(uint(sizeof(T))); }
        ComputeShaderBuffer CreateCSOutputBuffer(uint);

        template <class T>
        ComputeShaderBuffer CreateCSInputBuffer() { return CreateCSInputBuffer(uint(sizeof(T))); }
        ComputeShaderBuffer CreateCSInputBuffer(uint);

        template <typename T>
        Com<ID3D11Buffer> CreateCBuffer();

        void UpdateDynamicBuffer(ID3D11Resource* res, const void *data, size_t size)
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            HRESULT hr = ctx->Map(res, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (FAILED(hr))
            {
                // fuck.
                return;
            }
            memcpy(mapped.pData, data, size);
            ctx->Unmap(res, 0);
        }

        template <typename T>
        void UpdateDynamicBuffer(ID3D11Resource* res, const T& data)
        {
            UpdateDynamicBuffer(res, &data, sizeof(T));
        }

        Com<ID3D11Device1> device;
        Com<ID3D11DeviceContext1> ctx;
        Com<IDXGISwapChain> swapchain;

        RenderTarget backbuffer;

        Com<ID3D11Buffer> scratchVertex;
        Com<ID3D11SamplerState> sampler;

        //-----------------------------------------------------------------------------

        GlobalCBuffers cbuffers;

        void SetConstBuffer(int index, const Com<ID3D11Buffer>& buf, ShaderStages flags = VertexShader | PixelShader)
        {
            if (flags & VertexShader) ctx->VSSetConstantBuffers1(index, 1, &buf, nullptr, nullptr);
            if (flags & PixelShader) ctx->PSSetConstantBuffers1(index, 1, &buf, nullptr, nullptr);
        }

        template <typename T>
        void UploadConstBuffer(int index, const Com<ID3D11Buffer>& buf, const T& data, ShaderStages flags = VertexShader | PixelShader)
        {
            UpdateDynamicBuffer(buf.ptr(), &data, sizeof(T));
            SetConstBuffer(index, buf, flags);
        }

        //-----------------------------------------------------------------------------

        struct DepthStates {
            Com<ID3D11DepthStencilState> Default = nullptr;
            Com<ID3D11DepthStencilState> NoWrite;
            Com<ID3D11DepthStencilState> LessEqual;
            Com<ID3D11DepthStencilState> Ignore;
        } Depth;

        void SetDepthStencilState(const Com<ID3D11DepthStencilState>& dss, uint stencilRef = 0) {
            ctx->OMSetDepthStencilState(dss.ptr(), stencilRef);
        }

        //-----------------------------------------------------------------------------

        struct SampleStates {
            Com<ID3D11SamplerState> Default;
            Com<ID3D11SamplerState> Point;
        } Sample;

        void SetSampler(int slot, const Com<ID3D11SamplerState>& ss) {
            ctx->PSSetSamplers(slot, 1, &ss);
        }

        //-----------------------------------------------------------------------------

        struct RasterStates {
            Com<ID3D11RasterizerState> Default;
            Com<ID3D11RasterizerState> DepthBiased;
            Com<ID3D11RasterizerState> Wireframe;
            Com<ID3D11RasterizerState> SmoothLines;
        } Raster;

        void SetRasterState(const Com<ID3D11RasterizerState>& rs) {
            ctx->RSSetState(rs.ptr());
        }
    };
}
