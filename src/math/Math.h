#pragma once

#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_SWIZZLE

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <ostream>

namespace chisel
{
    // Standard vectors
    using vec4 = glm::vec4;
    using vec3 = glm::vec3;
    using vec2 = glm::vec2;

    // Float vectors
    using float4 = glm::vec4;
    using float3 = glm::vec3;
    using float2 = glm::vec2;

    // Signed integer vectors
    using int4 = glm::ivec4;
    using int3 = glm::ivec3;
    using int2 = glm::ivec2;

    // Unsigned integer vectors
    using uint4 = glm::uvec4;
    using uint3 = glm::uvec3;
    using uint2 = glm::uvec2;

    // Quaternion
    using quat = glm::quat;

    // Matrices
    using mat4x4 = glm::mat4x4;
    using mat3x3 = glm::mat3x3;
    using float4x4 = glm::mat4x4;
    using float3x3 = glm::mat3x3;

    enum struct Space {
        World,
        Local
    };

    // Defines coordinate space vectors for a
    // right-handed (+X forward), Z-up system
    inline struct Vectors
    {
        static const inline vec3 One  = vec3(1, 1, 1);
        static const inline vec3 Zero = vec3(0, 0, 0);

        // +X forward, -X backward
        static const inline vec3 Forward = vec3(+1, 0, 0);
        static const inline vec3 Back    = vec3(-1, 0, 0);

        // +Z up, -Z down
        static const inline vec3 Up   = vec3(0, 0, +1);
        static const inline vec3 Down = vec3(0, 0, -1);

        // -Y right, +Y left
        static const inline vec3 Right = vec3(0, +1, 0);
        static const inline vec3 Left  = vec3(0, -1, 0);
    } Vectors;

    namespace math
    {
        // Convert radians to degrees (scalar or vector)
        using glm::degrees;
        
        // Convert degrees to radians (scalar or vector)
        using glm::radians;

        static constexpr float EQUAL_EPSILON = 0.001f;

        inline bool CloseEnough(float a, float b, float epsilon = EQUAL_EPSILON)
        {
            return std::abs(a - b) <= epsilon;
        }

        inline bool CloseEnough(const vec3& a, const vec3& b, float epsilon = EQUAL_EPSILON)
        {
            return std::abs(a.x - b.x) <= epsilon &&
                std::abs(a.y - b.y) <= epsilon &&
                std::abs(a.z - b.z) <= epsilon;
        }

        inline float Snap(float value, float snap)
        {
            return std::round(value / snap) * snap;
        }

        inline glm::vec3 Snap(glm::vec3 value, glm::vec3 snap)
        {
            return glm::round(value / snap) * snap;
        }
    }

    struct Rect
    {
        union {
            struct { float x, y; };
            vec2 pos;
        };
        union {
            struct { float w, h; };
            struct { float width, height; };
            vec2 size;
        };

        Rect(auto x, auto y, auto w, auto h) : pos(float(x), float(y)), size(float(w), float(h)) {}
        Rect(vec2 pos, vec2 size) : pos(pos), size(size) {}
        Rect(vec4 vec) : pos(vec.xy), size(vec.zw) {}
        Rect() : pos(0), size(0) {}

        operator vec4() const { return vec4(pos, size); }

        vec2 Max() const { return pos + size; }
    };

}

namespace std
{
    template <auto N, class T, auto Q>
    struct tuple_size<glm::vec<N, T, Q>> : std::integral_constant<std::size_t, N> { };

    template <std::size_t I, auto N, class T, auto Q>
    struct tuple_element<I, glm::vec<N, T, Q>> {
        using type = decltype(get<I>(declval<glm::vec<N,T,Q>>()));
    };
}

namespace glm
{
    template <std::size_t I, auto N, class T, auto Q>
    constexpr auto& get(glm::vec<N, T, Q>& v) noexcept { return v[I]; }

    template <std::size_t I, auto N, class T, auto Q>
    constexpr const auto& get(const glm::vec<N, T, Q>& v) noexcept { return v[I]; }

    template <std::size_t I, auto N, class T, auto Q>
    constexpr auto&& get(glm::vec<N, T, Q>&& v) noexcept { return std::move(v[I]); }

    template <std::size_t I, auto N, class T, auto Q>
    constexpr const auto&& get(const glm::vec<N, T, Q>&& v) noexcept { return std::move(v[I]); }

    // Automatic GLM string casts.
    // fmtlib requires these be in namespace glm

    // Write vecN to stream
    template <int N, typename T, glm::qualifier P>
    inline std::ostream& operator<<(std::ostream& out, const glm::vec<N, T, P>& g) {
        return out << glm::to_string(g);
    }

    // Write matNxM to stream
    template <int N, int M, typename T, glm::qualifier P>
    inline std::ostream& operator<<(std::ostream& out, const glm::mat<N, M, T, P>& g) {
        return out << glm::to_string(g);
    }

    // Write quat to stream
    inline std::ostream& operator<<(std::ostream& out, const glm::quat& g) {
        return out << glm::to_string(g);
    }
}
