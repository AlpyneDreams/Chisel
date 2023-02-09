
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "assets/Assets.h"
#include "render/Texture.h"
#include "chisel/Tools.h"
#include "libvtf-plusplus/libvtf++.hpp"

#include <span>

namespace chisel
{
    static Texture* LoadTexture(std::string_view path, std::vector<uint8_t> data)
    {
        int width, height, channels;

        // 8 bits per channel
        std::unique_ptr<uint8_t[]> owned_data;
        owned_data.reset(stbi_load_from_memory(data.data(), int(data.size()), &width, &height, &channels, STBI_rgb_alpha));

        if (!owned_data)
            return nullptr;

        Texture* texture = new Texture(uint16_t(width), uint16_t(height), 1u, Texture::Format::RGBA8, std::move(owned_data), width * height * 4);
        texture->path = path;

        // This frees data when the upload completes.
        // TODO: This requires renderer to be initialized.
        // Ideally we should upload the texture on first use like with meshes.
        chisel::Tools.Render.UploadTexture(texture);

        return texture;
    }

    template <>
    Texture* ImportAsset<Texture, FixedString(".PNG")>(std::string_view path, std::vector<uint8_t> data) { return LoadTexture(path, std::move(data)); }

    template <>
    Texture* ImportAsset<Texture, FixedString(".TGA")>(std::string_view path, std::vector<uint8_t> data) { return LoadTexture(path, std::move(data)); }

    static render::TextureFormat RemapVTFImageFormat(libvtf::ImageFormat format) {
        switch (format) {
            case libvtf::ImageFormats::RGBA8888:       return render::TextureFormats::RGBA8;
            case libvtf::ImageFormats::BGRA8888:       return render::TextureFormats::BGRA8;
            case libvtf::ImageFormats::BGR565:         return render::TextureFormats::B5G6R5;
            case libvtf::ImageFormats::RGB565:         return render::TextureFormats::R5G6B5;
            case libvtf::ImageFormats::DXT1_RUNTIME: [[fallthrough]];
            case libvtf::ImageFormats::DXT1:           return render::TextureFormats::BC1;
            case libvtf::ImageFormats::DXT3:           return render::TextureFormats::BC2;
            case libvtf::ImageFormats::DXT5:           return render::TextureFormats::BC3;
            case libvtf::ImageFormats::R32F:           return render::TextureFormats::R32F;
            case libvtf::ImageFormats::RG3232F:        return render::TextureFormats::RG32F;
            case libvtf::ImageFormats::RGBA32323232F:  return render::TextureFormats::RGBA32F;
            default: throw std::runtime_error("Cannot remap format!");
        }
    }

    template <>
    Texture* ImportAsset<Texture, FixedString(".VTF")>(std::string_view path, std::vector<uint8_t> data) {
        // TODO: Make copy-less.
        libvtf::VTFData vtfData(data);

        const auto& header = vtfData.getHeader();
        std::span<const uint8_t> imageData = vtfData.imageData(0, 0, 0);

        std::unique_ptr<uint8_t[]> owned_data{ new uint8_t[imageData.size()] };
        std::memcpy(owned_data.get(), imageData.data(), imageData.size());

        Texture* texture = new Texture(uint16_t(header.width), uint16_t(header.height), uint16_t(header.depth), RemapVTFImageFormat(header.format), std::move(owned_data), imageData.size());
        texture->path = path;

        return nullptr;
    }
}