
#include "VMF.h"

#include "common/String.h"

#include "console/Console.h"
#include "math/Math.h"
#include <glm/gtx/normal.hpp>

namespace chisel
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
            float y = std::stof(std::string(coords[2]));
            float z = std::stof(std::string(coords[1]));

            // FIXME: Apply scale here for now
            tri[i] = vec3(x, y, z) * vec3(0.0254f);
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
        std::vector<vec3>& vertices = *new std::vector<vec3>();
        std::vector<uint32>& indices = *new std::vector<uint32>();
        uint32 index = 0;

        // Add each face to mesh
        for (auto& side : sides)
        {
            auto& tri = side.plane.tri;

            // Compute normal
            vec3 normal = glm::triangleNormal(tri[0], tri[1], tri[2]);

            // Add first triangle
            for (int i = 0; i < 3; i++)
            {
                vertices.push_back(tri[i]);
                vertices.push_back(normal);
                indices.push_back(index++);
            }

            // TODO: Actual face reconstruction by intersecting planes

            // Add second triangle
            indices.push_back(index - 3); // tri[0]
            indices.push_back(index - 1); // tri[2]

            vec3 dy = tri[1] - tri[0]; // top left - bottom left

            // Add 4th vertex
            vertices.push_back(tri[2] - dy);
            vertices.push_back(normal);
            indices.push_back(index++);
        }

        mesh = Mesh(xyz,
            &vertices[0].x, vertices.size() * sizeof(vec3),
            &indices[0], indices.size() * sizeof(uint32)
        );
    }

}