#pragma once

#include "Plane.h"

namespace chisel
{
    static constexpr uint32_t PlaneWindingPoints = 4;
    static constexpr uint32_t DefaultMaxWindingPoints = 128;

    template <uint32_t N>
    struct GenericWinding
    {
        static constexpr uint32_t MaxWindingPoints = N;

        vec3 points[MaxWindingPoints];
        uint32_t count = 0;

        static bool CreateFromPlane(const Plane& plane, GenericWinding& winding)
        {
            uint32_t x = ~0u;
            float max = -FLT_MAX;
            for (uint32_t i = 0; i < 3; i++)
            {
                float v = fabsf(plane.normal[i]);
                if (v > max)
                {
                    x = i;
                    max = v;
                }
            }

            if (x == ~0u)
            {
                return false;
            }

            vec3 up = vec3(0);
            switch (x)
            {
            case 0:
            case 1:
                up[2] = 1.0f;
                break;
            case 2:
                up[0] = 1.0f;
                break;
            }

            float v = glm::dot(up, plane.normal);
            up = glm::normalize(up + plane.normal * -v);

            vec3 org = plane.normal * plane.Dist();
            vec3 right = glm::cross(up, plane.normal);

            static constexpr float MaxTraceLength = 1.732050807569f * 32768.0f;

            up = up * MaxTraceLength;
            right = right * MaxTraceLength;

            winding.count = 4;
            winding.points[0] = (org - right) + up;
            winding.points[1] = (org + right) + up;
            winding.points[2] = (org + right) - up;
            winding.points[3] = (org - right) - up;

            return true;
        }

        static GenericWinding* Clip(const Plane& split, GenericWinding& inWinding, GenericWinding& scratchWinding)
        {
            static constexpr int SIDE_FRONT = 0;
            static constexpr int SIDE_BACK = 1;
            static constexpr int SIDE_ON = 2;

            static constexpr float SplitEpsilion = 0.01f;

            float dists[GenericWinding::MaxWindingPoints];
            int sides[GenericWinding::MaxWindingPoints];
            int counts[3] = { 0, 0, 0 };
            for (uint32_t i = 0; i < inWinding.count; i++)
            {
                vec3 point = inWinding.points[i];

                float dot = glm::dot(point, split.normal) - split.Dist();
                dists[i] = dot;

                int side;
                if (dot > SplitEpsilion)
                    side = SIDE_FRONT;
                else if (dot < -SplitEpsilion)
                    side = SIDE_BACK;
                else
                    side = SIDE_ON;
                sides[i] = side;

                counts[side]++;
            }
            sides[inWinding.count] = sides[0];
            dists[inWinding.count] = dists[0];

            if (!counts[SIDE_FRONT] && !counts[SIDE_BACK])
                return &inWinding;

            if (!counts[SIDE_FRONT])
                return nullptr;

            if (!counts[SIDE_BACK])
                return &inWinding;

            uint32_t maxPoints = inWinding.count + 4;
            assert(GenericWinding::MaxWindingPoints >= maxPoints);

            uint32_t numPoints = 0;
            for (uint32_t i = 0; i < inWinding.count; i++)
            {
                vec3* p1 = &inWinding.points[i];
                vec3* mid = &scratchWinding.points[numPoints];

                if (sides[i] == SIDE_FRONT || sides[i] == SIDE_ON)
                {
                    *mid = *p1;
                    numPoints++;
                    if (sides[i] == SIDE_ON)
                        continue;
                    mid = &scratchWinding.points[numPoints];
                }

                if (sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i])
                    continue;

                vec3* p2 = i == inWinding.count - 1
                    ? &inWinding.points[0]
                    : p1 + 1;

                numPoints++;

                float dot = dists[i] / (dists[i] - dists[i + 1]);
                for (uint32_t j = 0; j < 3; j++)
                {
                    if (split.normal[j] == 1)
                        (*mid)[j] = split.Dist();
                    else if (split.normal[j] == -1)
                        (*mid)[j] = -split.Dist();

                    (*mid)[j] = (*p1)[j] + dot * ((*p2)[j] - (*p1)[j]);
                }
            }

            scratchWinding.count = numPoints;
            return &scratchWinding;
        }
    };

    using PlaneWinding = GenericWinding<PlaneWindingPoints>;
    using Winding = GenericWinding<DefaultMaxWindingPoints>;
}
