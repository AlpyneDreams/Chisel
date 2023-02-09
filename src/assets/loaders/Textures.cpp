
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "assets/Assets.h"
#include "render/Texture.h"
#include "chisel/Tools.h"

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
}