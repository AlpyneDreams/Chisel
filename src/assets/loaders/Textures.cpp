
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "assets/Assets.h"
#include "render/Render.h"
#include "chisel/Tools.h"
#include "chisel/VMF/KeyValues.h"
#include "libvtf-plusplus/libvtf++.hpp"

#include <span>

namespace chisel
{
    static void LoadTexture(Texture& tex, const Buffer& data)
    {
        int width, height, channels;

        // 8 bits per channel
        std::unique_ptr<uint8_t[]> owned_data;
        owned_data.reset(stbi_load_from_memory(data.data(), int(data.size()), &width, &height, &channels, STBI_rgb_alpha));

        if (!owned_data)
            throw std::runtime_error("STB failed to load texture.");

        D3D11_TEXTURE2D_DESC desc =
        {
            .Width = UINT(width),
            .Height = UINT(height),
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = { 1, 0 },
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        };
        D3D11_SUBRESOURCE_DATA initialData =
        {
            .pSysMem = owned_data.get(),
            .SysMemPitch = UINT(width) * 4u,
            .SysMemSlicePitch = 0,
        };
        Tools.rctx.device->CreateTexture2D(&desc, &initialData, &tex.texture);
        Tools.rctx.device->CreateShaderResourceView(tex.texture.ptr(), nullptr, &tex.srv);
    }

    static AssetLoader<Texture, FixedString(".PNG")> PNGLoader = &LoadTexture;
    static AssetLoader<Texture, FixedString(".TGA")> TGALoader = &LoadTexture;

    static DXGI_FORMAT RemapVTFImageFormat(libvtf::ImageFormat format) {
        switch (format) {
        case libvtf::ImageFormats::RGBA8888:       return DXGI_FORMAT_R8G8B8A8_UNORM;
        case libvtf::ImageFormats::BGRA8888:       return DXGI_FORMAT_B8G8R8A8_UNORM;
        case libvtf::ImageFormats::BGR565:         return DXGI_FORMAT_B5G6R5_UNORM;
        case libvtf::ImageFormats::DXT1_RUNTIME: [[fallthrough]];
        case libvtf::ImageFormats::DXT1:           return DXGI_FORMAT_BC1_UNORM;
        case libvtf::ImageFormats::DXT3:           return DXGI_FORMAT_BC2_UNORM;
        case libvtf::ImageFormats::DXT5:           return DXGI_FORMAT_BC3_UNORM;
        case libvtf::ImageFormats::R32F:           return DXGI_FORMAT_R32_FLOAT;
        case libvtf::ImageFormats::RG3232F:        return DXGI_FORMAT_R32G32_FLOAT;
        case libvtf::ImageFormats::RGBA32323232F:  return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default: throw std::runtime_error("Cannot remap format!");
        }
    }

    inline std::pair<uint32_t, uint32_t> GetBlockSize(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC1_UNORM:
            return std::make_pair<uint32_t, uint32_t>(4, 4);
        default:
            return std::make_pair<uint32_t, uint32_t>(1, 1);
        }
    }

    inline uint32_t GetElementSize(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_B5G6R5_UNORM:
            return 2;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R32_FLOAT:
            return 4;
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_BC1_UNORM:
            return 8;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC3_UNORM:
            return 16;
        default:
            throw std::runtime_error("Cannot remap format!");
        }
    }

    static AssetLoader<Texture, FixedString(".VTF")> VTFLoader = [](Texture& tex, const Buffer& data)
    {
        // TODO: Make copy-less.
        libvtf::VTFData vtfData(data);

        const auto& header = vtfData.getHeader();
        std::span<const uint8_t> imageData = vtfData.imageData(0, 0, 0);

        Console.Log("Loading {}: {} {}", tex.GetPath(), (void*)imageData.data(), imageData.size());

        D3D11_TEXTURE2D_DESC desc =
        {
            .Width      = header.width,
            .Height     = header.height,
            .MipLevels  = 1,//header.numMipLevels,
            .ArraySize  = 1,//header.depth,
            .Format     = RemapVTFImageFormat(header.format),
            .SampleDesc = { 1, 0 },
            .Usage      = D3D11_USAGE_IMMUTABLE,
            .BindFlags  = D3D11_BIND_SHADER_RESOURCE,
        };
        // TODO: Mips
        // TODO Pitch proper
        D3D11_SUBRESOURCE_DATA initialData =
        {
            .pSysMem          = imageData.data(),
            .SysMemPitch      = (header.width / GetBlockSize(desc.Format).first) * GetElementSize(desc.Format),
            .SysMemSlicePitch = 0,
        };
        Tools.rctx.device->CreateTexture2D(&desc, &initialData, &tex.texture);
        Tools.rctx.device->CreateShaderResourceView(tex.texture.ptr(), nullptr, &tex.srv);
    };

    static AssetLoader <Material, FixedString(".VMT")> VMTLoader = [](Material& mat, const Buffer& data)
    {
        // TODO: This sucks.
        auto str = std::string((const char*)data.data(), data.size());
        auto r_kv = KeyValues::Parse(str);
        if (!r_kv)
            return;

        KeyValues &kv = *r_kv;

        KeyValues& basetexture = kv["$basetexture"];
        if (basetexture.type != KeyValues::Null)
        {
            std::string val = basetexture;

            // stupid bodge. using std string here sucks too. should just use fixed size strings of MAX_PATH on stack.
            if (!val.starts_with("materials"))
                val = "materials/" + val;
            if (!val.ends_with(".vtf"))
                val += ".vtf";
            mat.baseTexture = Assets.Load<Texture>(val);
        }
    };
}
