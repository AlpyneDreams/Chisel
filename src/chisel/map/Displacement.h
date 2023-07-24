#pragma once

#include "math/Math.h"

#include <vector>
#include <memory>

namespace chisel
{
    struct DispVert
    {
        vec3    normal;
        float   dist;
        vec3    offset;
        vec3    offsetNormal;
        float   alpha;
    };

    struct DispInfo
    {
        int                     power = 4;  // Subdivision power
        int                     length;     // Number of verts per side: (1 << power) + 1
        vec3                    startPos;
        float                   elevation;
        bool                    subdiv;
        int                     flags;
        std::vector<DispVert>   verts;
        int                     pointStartIndex = -1;

        DispInfo(int power)
          : power(power),
            length((1 << power) + 1)
        {
            verts.resize(length * length);
        }

        DispVert* operator[](int row)
        {
            return &verts[row * length];
        }

        uint GetIndexCount()
        {
            // 2^n x 2^m quads, 2 tris per quad, 3 verts per tri.
            int quadLength = length - 1;
            return (2 * quadLength * quadLength) * 3;
        }

        void UpdatePointStartIndex(const std::vector<vec3>& points)
        {
            if (pointStartIndex != -1)
                return;

            int minIndex = -1;
            float minDistanceSq = FLT_MAX;

            assert(points.size() == 4);
            for (int i = 0; i < 4; i++)
            {
                float distSq = glm::length2(startPos - points[i]);
                if (distSq < minDistanceSq)
                {
                    minDistanceSq = distSq;
                    minIndex = i;
                }
            }

            pointStartIndex = minIndex;
        }
    };
}
