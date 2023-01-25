#pragma once

#include <rain/rain.h>

namespace engine
{
    namespace refl = rain;

    static_assert(std::is_same_v<rain::Hash, std::uint32_t>, "[Rain] Hash is not uint32_t. May not be compatible with EnTT.");
}