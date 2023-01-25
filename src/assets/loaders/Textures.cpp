
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "assets/Assets.h"
#include "render/Texture.h"
#include "hammer/Tools.h"

#include <span>

namespace engine
{
    static Texture* LoadTexture(const fs::Path& path)
    {
        int x, y, n;

        // 8 bits per channel
        byte* data = stbi_load(path.string().c_str(), &x, &y, &n, 0);

        if (!data)
            return nullptr;

        Texture* texture = new Texture(uint(x), uint(y), data, uint(n), 8);
        texture->path = path;

        // This frees data when the upload completes.
        // TODO: This requires renderer to be initialized.
        // Ideally we should upload the texture on first use like with meshes.
        editor::Tools.Render.UploadTexture(texture);

        return texture;
    }

    template <>
    Texture* ImportAsset<Texture, FixedString(".PNG")>(const fs::Path& path) { return LoadTexture(path); }

    template <>
    Texture* ImportAsset<Texture, FixedString(".TGA")>(const fs::Path& path) { return LoadTexture(path); }
}