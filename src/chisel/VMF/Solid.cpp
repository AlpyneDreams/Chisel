
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
    {}


    Solid::Solid(KeyValues& solid) : MapClass(solid),
        sides(solid["side"])
    {
    }

}