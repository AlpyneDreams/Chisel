#include "assets/Assets.h"
#include "render/Render.h"
#include "formats/KeyValues.h"

namespace chisel
{
    static Rc<Texture> LoadVTF(std::string_view name)
    {
        std::string val = std::string((std::string_view)name);

        // stupid bodge. using std string here sucks too. should just use fixed size strings of MAX_PATH on stack.
        if (!val.starts_with("materials"))
            val = "materials/" + val;
        if (!val.ends_with(".vtf"))
            val += ".vtf";

        return Assets.Load<Texture>(val);
    }

    static AssetLoader <Material, FixedString(".VMT")> VMTLoader = [](Material& mat, const Buffer& data)
    {
        auto r_kv = kv::KeyValues::ParseFromUTF8(chisel::StringView(data));
        if (!r_kv)
            return;

        if (r_kv->begin() == r_kv->end())
            return;

        // Get past the root member.
        kv::KeyValues &kv = r_kv->begin()->second.front();

        if (auto& basetexture = kv["$basetexture"])
            mat.baseTexture = LoadVTF((std::string_view)basetexture);

        if (auto& basetexture2 = kv["$basetexture2"])
            mat.baseTextures[0] = LoadVTF((std::string_view)basetexture2);

        mat.translucent = kv["$translucent"];
        mat.alphatest = kv["$alphatest"];
    };

}
