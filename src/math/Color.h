#pragma once

#include <limits>
#include <algorithm>
#include <type_traits>

#include "common/Common.h"
#include "math/Math.h"

namespace chisel
{
    template <typename T = float>
    struct ColorRGBA
    {
        static constexpr T NormalMin = T(0);
        static constexpr T NormalMax = std::is_floating_point<T>::value ? T(1.0) : std::numeric_limits<T>::max();

        union {
            T data[4];
            struct {
                T r, g, b, a;
            };
        };

        ColorRGBA() : ColorRGBA(NormalMin, NormalMin, NormalMin) {}

        explicit ColorRGBA(T r, T g, T b, T a = NormalMax)
            : r(r), g(g), b(b), a(a) {}

        explicit ColorRGBA(double r, double g, double b, double a = NormalMax)
            : r(r), g(g), b(b), a(a) {}

        explicit ColorRGBA(auto r, auto g, auto b, auto a = NormalMax)
            : r(T(r)), g(T(g)), b(T(b)), a(T(a)) {}

        explicit ColorRGBA(auto r, auto g, auto b)
            : r(T(r)), g(T(g)), b(T(b)), a(T(NormalMax)) {}
        
        static ColorRGBA<T> HSV(double H, double S, double V)
        {
            ColorRGBA<T> RGB;
            double P, Q, U, fract;
            (H == 360.) ? (H = 0.) : (H /= 60.);
            fract = H - floor(H);
            P = V*(1. - S);
            Q = V*(1. - S*fract);
            U = V*(1. - S*(1. - fract));
            if      (0. <= H && H < 1.)
                RGB = ColorRGBA<T>{V, U, P};
            else if (1. <= H && H < 2.)
                RGB = ColorRGBA<T>{Q, V, P};
            else if (2. <= H && H < 3.)
                RGB = ColorRGBA<T>{P, V, U};
            else if (3. <= H && H < 4.)
                RGB = ColorRGBA<T>{P, Q, V};
            else if (4. <= H && H < 5.)
                RGB = ColorRGBA<T>{U, P, V};
            else if (5. <= H && H < 6.)
                RGB = ColorRGBA<T>{V, P, Q};
            else
                RGB = ColorRGBA<T>{0., 0., 0.};
            return RGB;
        }
        
        // Pack RGBA
        uint32 Pack()
        {
            using std::clamp;
            constexpr T packScale = T(255) / NormalMax;
            return    uint32( clamp(r * packScale, T{0}, T{255}) ) << 24u
                    | uint32( clamp(g * packScale, T{0}, T{255}) ) << 16u
                    | uint32( clamp(b * packScale, T{0}, T{255}) ) <<  8u
                    | uint32( clamp(a * packScale, T{0}, T{255}) ) <<  0u ;
        }
        
        uint32 PackABGR()
        {
            using std::clamp;
            constexpr T packScale = T(255) / NormalMax;
            return    uint32( clamp(a * packScale, T{0}, T{255}) ) << 24u
                    | uint32( clamp(b * packScale, T{0}, T{255}) ) << 16u
                    | uint32( clamp(g * packScale, T{0}, T{255}) ) <<  8u
                    | uint32( clamp(r * packScale, T{0}, T{255}) ) <<  0u ;
        }

        operator vec4() const
        {
            return vec4(
                float(r) / float(NormalMax),
                float(g) / float(NormalMax),
                float(b) / float(NormalMax),
                float(a) / float(NormalMax)
            );
        }

        operator const float*() const
        {
            return &data[0];
        }
    };

    using Color   = ColorRGBA<float>;
    using Color255 = ColorRGBA<byte>;

    // List of common colors
    const inline struct {
        Color Transparent   = Color(0, 0, 0, 0);
        Color Black         = Color(0, 0, 0, 1);
        Color White         = Color(1, 1, 1, 1);
        Color Green         = Color(0, 1, 0, 1);
    } Colors;

}