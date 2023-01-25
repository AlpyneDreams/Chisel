#pragma once

namespace chisel::render
{
    enum class BlendMode
    {
        Default,
        Zero, One,
        SrcColor, OneMinusSrcColor,
        DstColor, OneMinusDstColor,
        SrcAlpha, OneMinusSrcAlpha,
        DstAlpha, OneMinusDstAlpha,
        SrcAlphaSaturate
    };

    enum class BlendOp
    {
        Default, Add, Subtract, ReverseSubtract, Min, Max
    };

    struct BlendFunc
    {
        BlendMode src, dst, srcAlpha, dstAlpha;
        BlendOp rgbOp = BlendOp::Default;
        BlendOp alphaOp = BlendOp::Default;

        constexpr BlendFunc(BlendMode src, BlendMode dst)
          : src(src), dst(dst), srcAlpha(src), dstAlpha(dst) {}

        constexpr BlendFunc() : BlendFunc(BlendMode::Default, BlendMode::Default) {}
    };

    struct BlendFuncs
    {
        using enum BlendMode;
        static constexpr BlendFunc Normal   = BlendFunc();
        static constexpr BlendFunc Add      = BlendFunc(One, One);
        static constexpr BlendFunc Alpha    = BlendFunc(SrcAlpha, OneMinusSrcAlpha);
    };

}