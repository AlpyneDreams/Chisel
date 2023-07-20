#include "assets/Assets.h"
#include "render/Render.h"
#include "formats/KeyValues.h"

namespace chisel
{
    static Texture* LoadVTF(std::string_view name)
    {
        std::string val = std::string((std::string_view)name);

        // stupid bodge. using std string here sucks too. should just use fixed size strings of MAX_PATH on stack.
        if (!val.starts_with("materials"))
            val = "materials/" + val;
        if (!val.ends_with(".vtf"))
            val += ".vtf";
        Texture* tex = Assets.Load<Texture>(val);
        if (!tex) // TODO: Use MapRender.Textures.Missing
            return Assets.Load<Texture>("textures/error.png");
        else
            return tex;
    }

    static AssetLoader <Material, FixedString(".VMT")> VMTLoader = [](Material& mat, const Buffer& data)
    {
        auto r_kv = kv::KeyValues::ParseFromUTF8(chisel::StringView(data));
        if (!r_kv)
            return;

        if (r_kv->begin() == r_kv->end())
            return;

        // Get past the root member.
        kv::KeyValues &kv = r_kv->begin()->second;

        if (auto& basetexture = kv["$basetexture"])
            mat.baseTexture = LoadVTF((std::string_view)basetexture);
        else // TODO: Use MapRender.Textures.Missing
            mat.baseTexture = Assets.Load<Texture>("textures/error.png");

        if (auto& basetexture2 = kv["$basetexture2"])
            mat.baseTextures[0] = LoadVTF((std::string_view)basetexture2);

        mat.translucent = kv["$translucent"];
        mat.alphatest = kv["$alphatest"];
    };

}
