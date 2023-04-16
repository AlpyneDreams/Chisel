#include "render/Render.h"

#include <imgui.h>
#include "gui/Common.h"
#include "gui/impl/imgui_impl_dx11.h"
#include "common/Filesystem.h"
#include "core/Mesh.h"
#include "console/Console.h"
#include "render/CBuffers.h"
#include "render/TextureFormat.h"

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

        window->SetResizeCallback([this](uint width, uint height)
        {
            backbuffer.rtv = nullptr;
            backbuffer.texture = nullptr;
            HRESULT hr = swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
            if (FAILED(hr))
                Console.Error("Failed to resize swapchain to {} x {} (is something using the backbuffer?)", width, height);
            swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer.texture));
            device->CreateRenderTargetView(backbuffer.texture.ptr(), nullptr, &backbuffer.rtv);
        });

        GUI::Setup();

        ImGui_ImplDX11_Init(device.ptr(), ctx.ptr());
        ImGui_ImplDX11_NewFrame();

        // Scratch Vertex Buffer
        D3D11_BUFFER_DESC bufferDesc = {
            .ByteWidth      = 4 * 1024,
            .Usage          = D3D11_USAGE_DYNAMIC,
            .BindFlags      = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER,
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
        };
        hr = device->CreateBuffer(&bufferDesc, nullptr, &scratchVertex);
        if (FAILED(hr))
            Console.Error("[D3D11] Failed to create scratch vertex buffer.");

        // Global CBuffers
        cbuffers.camera = CreateCBuffer<cbuffers::CameraState>();
        cbuffers.object = CreateCBuffer<cbuffers::ObjectState>();
        cbuffers.brush  = CreateCBuffer<cbuffers::BrushState>();

        // Global blend states
        CreateBlendState(BlendFuncs::Normal);
        CreateBlendState(BlendFuncs::Add);
        CreateBlendState(BlendFuncs::Alpha);

        // Global depth stencil states
        D3D11_DEPTH_STENCIL_DESC dssDefault = { // Same as you'd get with nullptr
            .DepthEnable = TRUE,
            .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D11_COMPARISON_LESS,
            .StencilEnable = FALSE,
            .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
            .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
            .FrontFace =
            {
                .StencilFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilPassOp = D3D11_STENCIL_OP_KEEP,
                .StencilFunc = D3D11_COMPARISON_ALWAYS,
            },
            .BackFace =
            {
                .StencilFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilPassOp = D3D11_STENCIL_OP_KEEP,
                .StencilFunc = D3D11_COMPARISON_ALWAYS,
            },
        };

        D3D11_DEPTH_STENCIL_DESC dssNoWrite = dssDefault;
        dssNoWrite.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        device->CreateDepthStencilState(&dssNoWrite, &Depth.NoWrite);

        D3D11_DEPTH_STENCIL_DESC dssLessEqual = dssDefault;
        dssLessEqual.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        device->CreateDepthStencilState(&dssLessEqual, &Depth.LessEqual);

        D3D11_DEPTH_STENCIL_DESC dssIgnore = dssDefault;
        dssIgnore.DepthEnable = FALSE;
        dssIgnore.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dssIgnore.DepthFunc = D3D11_COMPARISON_NEVER;
        device->CreateDepthStencilState(&dssIgnore, &Depth.Ignore);

        // Global samplers
        D3D11_SAMPLER_DESC samplerDesc =
        {
            .Filter = D3D11_FILTER_ANISOTROPIC,
            .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
            .MipLODBias = 0.0f,
            .MaxAnisotropy = 16,
            .ComparisonFunc = D3D11_COMPARISON_ALWAYS,
            .MinLOD = 0.0f,
            .MaxLOD = FLT_MAX,
        };
        device->CreateSamplerState(&samplerDesc, &Sample.Default);
        ctx->PSSetSamplers(0, 1, &Sample.Default);

        D3D11_SAMPLER_DESC pointDesc = samplerDesc;
        pointDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        device->CreateSamplerState(&pointDesc, &Sample.Point);

        D3D11_RASTERIZER_DESC desc =
        {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_BACK,
            .FrontCounterClockwise = TRUE,
            .DepthClipEnable = TRUE,
        };
        device->CreateRasterizerState(&desc, &Raster.Default);
        ctx->RSSetState(Raster.Default.ptr());

        desc.AntialiasedLineEnable = TRUE;
        device->CreateRasterizerState(&desc, &Raster.SmoothLines);

        window->OnAttach();
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
        // Bind the backbuffer
        ctx->OMSetRenderTargets(1, &backbuffer.rtv, nullptr);

        uint2 size = backbuffer.GetSize();
        D3D11_VIEWPORT viewport =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width    = float(size.x),
            .Height   = float(size.y),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        ctx->RSSetViewports(1, &viewport);

        // Clear entire backbuffer
        float debugColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
        ctx->ClearRenderTargetView(backbuffer.rtv.ptr(), debugColor);
    }

    void RenderContext::EndFrame()
    {
        // Bind the backbuffer
        ctx->OMSetRenderTargets(1, &backbuffer.rtv, nullptr);

        uint2 size = backbuffer.GetSize();
        D3D11_VIEWPORT viewport =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width    = float(size.x),
            .Height   = float(size.y),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        ctx->RSSetViewports(1, &viewport);

        // Update ImGui
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        swapchain->Present(0, 0);
    }

    RenderTarget RenderContext::CreateRenderTarget(uint width, uint height, DXGI_FORMAT format)
    {
        RenderTarget rt;
        D3D11_TEXTURE2D_DESC rtDesc =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = LinearToTypeless(format),
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
        device->CreateTexture2D(&rtDesc, nullptr, &rt.texture);
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc =
        {
            .Format = LinearToSRGB(format),
            .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
            .Texture2D =
            {
                .MipSlice = 0,
            },
        };
        device->CreateRenderTargetView(rt.texture.ptr(), &rtvDesc, &rt.rtv);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDescLinear =
        {
            .Format = format,
            .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
            .Texture2D =
            {
                .MostDetailedMip = 0,
                .MipLevels = UINT(-1),
            },
        };
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDescSRGB =
        {
            .Format = LinearToSRGB(format),
            .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
            .Texture2D =
            {
                .MostDetailedMip = 0,
                .MipLevels = UINT(-1),
            },
        };
        device->CreateShaderResourceView(rt.texture.ptr(), &srvDescLinear, &rt.srvLinear);
        device->CreateShaderResourceView(rt.texture.ptr(), &srvDescSRGB, &rt.srvSRGB);
        return rt;
    }

    DepthStencil RenderContext::CreateDepthStencil(uint width, uint height, DXGI_FORMAT format)
    {
        DepthStencil ds;
        D3D11_TEXTURE2D_DESC dsDesc =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = format,
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
        device->CreateTexture2D(&dsDesc, nullptr, &ds.texture);
        device->CreateDepthStencilView(ds.texture.ptr(), nullptr, &ds.dsv);
        return ds;
    }

    void RenderContext::CreateBlendState(const BlendState& state)
    {
        if (state.handle == nullptr && state.Enabled())
        {
            D3D11_BLEND_DESC1 desc = {
                .AlphaToCoverageEnable = state.alphaToCoverage,
                .IndependentBlendEnable = state.independent
            };
            for (uint i = 0; i < 8; i++)
            {
                BlendFunc func = state.renderTargets[i];
                desc.RenderTarget[i] = {
                    .BlendEnable    = func.Enabled(),
                    .LogicOpEnable  = false,
                    .SrcBlend       = D3D11_BLEND(func.src),
                    .DestBlend      = D3D11_BLEND(func.dst),
                    .BlendOp        = D3D11_BLEND_OP(func.rgbOp),
                    .SrcBlendAlpha  = D3D11_BLEND(func.srcAlpha),
                    .DestBlendAlpha = D3D11_BLEND(func.dstAlpha),
                    .BlendOpAlpha   = D3D11_BLEND_OP(func.alphaOp),
                    .RenderTargetWriteMask = func.writeMask.mask
                };
            }
            device->CreateBlendState1(&desc, &state.handle);
        }
    }

    void RenderContext::SetBlendState(const BlendState& state, vec4 factor, uint32 sampleMask)
    {
        CreateBlendState(state);
        ctx->OMSetBlendState(state.handle.ptr(), &factor.x, sampleMask);
    }

    //--------------------------------------------------
    //  ComputeShaderBuffer
    //--------------------------------------------------

    void ComputeShaderBuffer::AddStagingBuffer(ID3D11Device1* device)
    {
        D3D11_BUFFER_DESC desc;
        buffer->GetDesc(&desc);
        D3D11_BUFFER_DESC bufferDesc = {
            .ByteWidth = desc.ByteWidth,
            .Usage = D3D11_USAGE_STAGING,
            .BindFlags = 0,
            .CPUAccessFlags = D3D11_CPU_ACCESS_READ,
            .MiscFlags = 0
        };
        HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &stagingBuffer);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create compute shader staging buffer");
        }
    }

    void ComputeShaderBuffer::QueueDownload()
    {
        ID3D11Device* device;
        buffer->GetDevice(&device);
        if (stagingBuffer == nullptr)
            AddStagingBuffer((ID3D11Device1*)device);
        ID3D11DeviceContext* ctx;
        device->GetImmediateContext(&ctx);
        ctx->CopyResource(stagingBuffer.ptr(), buffer.ptr());
    }

    void ComputeShaderBuffer::Download(void callback(void*))
    {
        ID3D11Device* device;
        buffer->GetDevice(&device);
        if (stagingBuffer == nullptr)
            AddStagingBuffer((ID3D11Device1*)device);
        ID3D11DeviceContext* ctx;
        device->GetImmediateContext(&ctx);
        ctx->CopyResource(stagingBuffer.ptr(), buffer.ptr());

        // Unfortunately, we need to wait here. There is no way to fence. (TODO: Really?)
        // We could potentially wait a frame or two if this causes hangs.
        D3D11_MAPPED_SUBRESOURCE map; 
        if (FAILED(ctx->Map(stagingBuffer.ptr(), 0, D3D11_MAP_READ, 0, &map)))
            return;

        callback(map.pData);
        ctx->Unmap(stagingBuffer.ptr(), 0);
    }

    ComputeShaderBuffer RenderContext::CreateCSOutputBuffer(uint size)
    {
        ComputeShaderBuffer rwsb;
        D3D11_BUFFER_DESC bufferDesc = {
            .ByteWidth = size,
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_UNORDERED_ACCESS,
            .CPUAccessFlags = 0,
        };
        HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &rwsb.buffer);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create compute shader output buffer");
            return rwsb;
        }

        D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {
            .Format = DXGI_FORMAT_R32_UINT,
            .ViewDimension = D3D11_UAV_DIMENSION_BUFFER,
            .Buffer = {
                .FirstElement = 0,
                .NumElements = size / sizeof(uint),
                .Flags = 0,
            }
        };
        hr = device->CreateUnorderedAccessView(rwsb.buffer.ptr(), &desc, &rwsb.uav);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create compute shader output buffer view");
            return rwsb;
        }

        return rwsb;
    }

    ComputeShaderBuffer RenderContext::CreateCSInputBuffer(uint size)
    {
        ComputeShaderBuffer rwsb;
        D3D11_BUFFER_DESC bufferDesc = {
            .ByteWidth = size, // * count
            .Usage = D3D11_USAGE_DYNAMIC,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
            .MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
            .StructureByteStride = size,
        };
        HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &rwsb.buffer);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create compute shader input buffer");
            return rwsb;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX,
            .BufferEx = {
                .FirstElement = 0,
                .NumElements = 1, // count 
                .Flags = 0,
            }
        };
        hr = device->CreateShaderResourceView(rwsb.buffer.ptr(), &srvd, &rwsb.srv);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create compute shader input buffer view");
            return rwsb;
        }

        return rwsb;
    }

    //--------------------------------------------------
    //  CBuffers
    //--------------------------------------------------

    template <typename T>
    Com<ID3D11Buffer> RenderContext::CreateCBuffer()
    {
        Com<ID3D11Buffer> buffer;
        D3D11_BUFFER_DESC bufferDesc = {
            .ByteWidth = sizeof(T),
            .Usage = D3D11_USAGE_DYNAMIC,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
        };
        HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &buffer);
        if (FAILED(hr))
            Console.Error("[D3D11] Failed to create constant buffer {}", typeid(T).name());
        return buffer;
    }

    //--------------------------------------------------
    //  DrawMesh
    //--------------------------------------------------

    void RenderContext::DrawMesh(Mesh* mesh)
    {
        if (!mesh->uploaded) {
            for (auto& group : mesh->groups) {
                D3D11_BUFFER_DESC desc = {};
                desc.ByteWidth  = group.vertices.Size();
                desc.Usage      = D3D11_USAGE_DEFAULT;
                desc.BindFlags  = D3D11_BIND_VERTEX_BUFFER;

                D3D11_SUBRESOURCE_DATA data = {};
                data.pSysMem    = group.vertices.pointer;

                HRESULT hr = device->CreateBuffer(&desc, &data, (ID3D11Buffer**)&group.vertices.handle);
                if (FAILED(hr)) {
                    Console.Error("[D3D11] Failed to create vertex buffer");
                    return;
                }
            }
        }
        uint strides[] = {(uint)mesh->groups[0].vertices.Stride()};
        uint offsets[] = {0};
        ctx->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&mesh->groups[0].vertices.handle, strides, offsets);
        ctx->Draw(mesh->groups[0].vertices.count, 0);
    }

    //--------------------------------------------------
    //  ComputeShader
    //--------------------------------------------------

    ComputeShader::ComputeShader(ID3D11Device1* device, std::string_view name)
    {
        fs::Path path = fs::Path("core/shaders") / name;
        path.setExt(".csc");
        auto csFile = fs::readFile(path);
        if (!csFile) {
            Console.Error("[D3D11] Failed to find compute shader 'shaders/{}.csc'", name);
            return;
        }

        HRESULT hr = device->CreateComputeShader(csFile->data(), csFile->size(), NULL, &cs);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create compute shader '{}'", name);
            return;
        }
    }

    //--------------------------------------------------
    //  Shader
    //--------------------------------------------------

    void RenderContext::SetShader(const Shader& shader)
    {
        if (shader.inputLayout != nullptr)
            ctx->IASetInputLayout(shader.inputLayout.ptr());
        if (shader.vs != nullptr)
            ctx->VSSetShader(shader.vs.ptr(), nullptr, 0);
        if (shader.ps != nullptr)
            ctx->PSSetShader(shader.ps.ptr(), nullptr, 0);
    }

    Shader::Shader(ID3D11Device1* device, Span<D3D11_INPUT_ELEMENT_DESC const> ia, std::string_view name)
    {
        fs::Path path = fs::Path("core/shaders") / name;
        path.setExt(".vsc");
        auto vsFile = fs::readFile(path);
        if (!vsFile) {
            Console.Error("[D3D11] Failed to find vertex shader 'shaders/{}.vsc'", name);
            return;
        }

        path.setExt(".psc");
        auto psFile = fs::readFile(path);
        if (!psFile) {
            Console.Error("[D3D11] Failed to find pixel shader 'shaders/{}.psc'", name);
            return;
        }

        HRESULT hr = device->CreateVertexShader(vsFile->data(), vsFile->size(), NULL, &vs);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create vertex shader '{}'", name);
            return;
        }

        hr = device->CreatePixelShader(psFile->data(), psFile->size(), NULL, &ps);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create pixel shader '{}'", name);
            return;
        }

        hr = device->CreateInputLayout(ia.data, ia.size, vsFile->data(), vsFile->size(), &inputLayout);
        if (FAILED(hr)) {
            Console.Error("[D3D11] Failed to create input layout for shader '{}'", name);
            return;
        }
    }

}
