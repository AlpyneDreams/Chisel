#pragma once

#include "common/Common.h"
#include "D3D11Include.h"

namespace chisel::render
{
    enum class BlendMode // D3D11_BLEND 
    {
        Default,
        Zero, One,
        SrcColor, OneMinusSrcColor,
        SrcAlpha, OneMinusSrcAlpha,
        DstAlpha, OneMinusDstAlpha,
        DstColor, OneMinusDstColor,
        SrcAlphaSaturate,
        BlendFactor = 14, OneMinusBlendFactor,
        Src1Color, OneMinusSrc1Color,
        Src1Alpha, OneMinusSrc1Alpha
    };

    enum class BlendOp // D3D11_BLEND_OP
    {
        Add = 1, Subtract, ReverseSubtract, Min, Max
    };

    struct RGBAMask // D3D11_COLOR_WRITE_ENABLE
    {
        union {
            struct {
                bool r : 1;
                bool g : 1;
                bool b : 1;
                bool a : 1;
            };
            byte mask : 4 = 0b1111;
        };

        constexpr RGBAMask() = default;
        constexpr RGBAMask(bool r, bool g, bool b, bool a) : r(r), g(g), b(b), a(a) {}
        constexpr RGBAMask(byte mask) : mask(mask) {}
    };

    struct BlendFunc // D3D11_RENDER_TARGET_BLEND_DESC
    {
        BlendMode src, dst, srcAlpha, dstAlpha;
        BlendOp rgbOp = BlendOp::Add;
        BlendOp alphaOp = BlendOp::Add;
        RGBAMask writeMask = 0b1111;

        constexpr BlendFunc(BlendMode src, BlendMode dst) : src(src), dst(dst), srcAlpha(src), dstAlpha(dst) {}
        constexpr BlendFunc() : BlendFunc(BlendMode::Default, BlendMode::Default) {}

        constexpr bool Enabled() const { return src != BlendMode::Default; }
    };

    struct BlendState // D3D11_BLEND_DESC
    {
        bool alphaToCoverage = false;
        bool independent = false;
        BlendFunc renderTargets[8];
        mutable Com<ID3D11BlendState1> handle = nullptr;

        BlendState() = default;
        BlendState(std::nullptr_t) {}
        BlendState(BlendFunc func) : renderTargets {func} {}
        BlendState(auto... funcs) : independent(true), renderTargets {funcs...} {}

        constexpr bool Enabled() const { return renderTargets[0].Enabled() || independent; }
    };

    class BlendFuncs
    {
        using enum BlendMode;
    public:
        static inline BlendState Normal   = BlendFunc();
        static inline BlendState Add      = BlendFunc(One, One);
        static inline BlendState Alpha    = BlendFunc(SrcAlpha, OneMinusSrcAlpha);
    };
}