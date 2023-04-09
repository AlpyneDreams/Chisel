#pragma once

#include "render/Render.h"
#include "Assets.h"

namespace chisel
{
    struct TextureAsset : public render::Texture, public Asset
    {};
}
