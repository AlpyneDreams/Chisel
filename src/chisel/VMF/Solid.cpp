
#include "VMF.h"

#include "common/String.h"

#include "console/Console.h"
#include "math/Math.h"
#include <glm/gtx/normal.hpp>

namespace chisel::VMF
{
    static VertexLayout xyz = {
        VertexAttribute::For<float>(3, VertexAttribute::Position),
        VertexAttribute::For<float>(3, VertexAttribute::Normal, true),
    };

    Plane::Plane(std::string_view plane)
    {
        auto points = str::split(plane, ")");

        // Parse points
        for (int i = 0; i < 3; i++)
        {
            // TODO: Why can't we remove the '(' with str::trim
            auto xyz = str::trim(points[i]);
            xyz.remove_prefix(1);

            auto coords = str::split(xyz, " ");
            // Convert z-up to y-up
            // TODO: Handedness?
            float x = std::stof(std::string(coords[0]));
            float y = std::stof(std::string(coords[1]));
            float z = std::stof(std::string(coords[2]));

            // FIXME: Apply scale here for now
            point_trio[i] = vec3(x, y, z);
        }
    }

    Side::Side(KeyValues& side) : MapAtom(side),
        plane               (side["plane"]),
        material            (side["material"]),
        rotation            (side["rotation"]),
        lightmapscale       (side["lightmapscale"]),
        smoothing_groups    (side["smoothing_groups"])
    {
        auto parseAxis = [&](std::string_view value, vec4& axis, float& scale)
        {
            auto values = str::split(value, "]");

            auto axis_part = values[0];
            axis_part.remove_prefix(1);
            axis_part = str::trim(axis_part);
            auto coords = str::split(axis_part, " ");
            float x = std::stof(std::string(coords[0]));
            float y = std::stof(std::string(coords[1]));
            float z = std::stof(std::string(coords[2]));
            float w = std::stof(std::string(coords[3]));
            axis = vec4(x, y, z, w);

            auto scale_part = values[1];
            scale_part.remove_prefix(1);
            scale_part = str::trim(scale_part);
            scale = std::stof(std::string(scale_part));
        };
        parseAxis(side["uaxis"], axis[0], scale[0]);
        parseAxis(side["vaxis"], axis[1], scale[1]);
    }


    Solid::Solid(KeyValues& solid) : MapClass(solid)
    {
        auto range = solid.FindAll("side");
        while (range.first != range.second)
        {
            sides.emplace_back(range.first->second);
            range.first++;
        }
    }

}