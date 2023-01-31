
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
        int x, y, n;

        // 8 bits per channel
        byte* texture_data = stbi_load_from_memory(data.data(), int(data.size()), &x, &y, &n, 0);

        if (!texture_data)
            return nullptr;

        Texture* texture = new Texture(uint(x), uint(y), texture_data, uint(n), 8);
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