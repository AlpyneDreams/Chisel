#pragma once

#include "math/Math.h"
#include "math/Plane.h"

namespace chisel
{
    namespace Orientations
    {
        enum Orientation : uint32_t
        {
            Floor = 0,
            Ceiling,
            NorthWall,
            SouthWall,
            EastWall,
            WestWall,

            Count,
            Invalid = Count,
            CountWithInvalid,

        };

        static const vec3 FaceNormals[CountWithInvalid] =
        {
            vec3( 0.0f,  0.0f,  1.0f ),
            vec3( 0.0f,  0.0f, -1.0f ),
            vec3( 0.0f, -1.0f,  0.0f ),
            vec3( 0.0f,  1.0f,  0.0f ),
            vec3(-1.0f,  0.0f,  0.0f ),
            vec3( 1.0f,  0.0f,  0.0f ),
            vec3( 0.0f ),
        };

        static const vec3 DownVectors[CountWithInvalid] =
        {
            vec3( 0.0f, -1.0f,  0.0f ),
            vec3( 0.0f, -1.0f,  0.0f ),
            vec3( 0.0f,  0.0f, -1.0f ),
            vec3( 0.0f,  0.0f, -1.0f ),
            vec3( 0.0f,  0.0f, -1.0f ),
            vec3( 0.0f,  0.0f, -1.0f ),
            vec3( 0.0f ),
        };

        static const vec3 RightVectors[CountWithInvalid] =
        {
            vec3( 1.0f,  0.0f,  0.0f ),
            vec3( 1.0f,  0.0f,  0.0f ),
            vec3( 1.0f,  0.0f,  0.0f ),
            vec3( 1.0f,  0.0f,  0.0f ),
            vec3( 0.0f,  1.0f,  0.0f ),
            vec3( 0.0f,  1.0f,  0.0f ),
            vec3( 0.0f ),
        };

        inline Orientation CalcOrientation(const Plane& plane)
        {
            if (plane.normal == vec3(0.0f))
                return Invalid;

            vec3 normal = glm::normalize(plane.normal);

            Orientation orientation = Invalid;
            float maxDot = 0.0f;
            for (uint32_t i = 0; i < 6; i++)
            {
                float dot = glm::dot(normal, FaceNormals[i]);
                if (dot >= maxDot)
                {
                    maxDot = dot;
                    orientation = (Orientation)i;
                }
            }
            return orientation;
        }
    }
    using Orientation = Orientations::Orientation;
}
