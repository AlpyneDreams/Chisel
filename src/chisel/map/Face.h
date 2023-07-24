#pragma once

#include "console/ConVar.h"
#include "render/Render.h"
#include "Orientation.h"
#include "Displacement.h"

#include <array>

namespace chisel
{
    // TODO: Move to some transform state?
    extern ConVar<bool>  trans_texture_lock;
    extern ConVar<bool>  trans_texture_scale_lock;
    extern ConVar<bool>  trans_texture_face_alignment;

    struct Side
    {
        Side()
        {
        }

        Side(const Plane& plane, Rc<Material> material, float scale = 0.25f)
            : plane{ plane }
            , material{ std::move(material) }
            , scale{ scale, scale }
        {
            Orientation orientation = Orientations::CalcOrientation(plane);
            if (orientation == Orientations::Invalid)
                return;

            textureAxes[1].xyz = Orientations::DownVectors[orientation];
            if (trans_texture_face_alignment)
            {
                // Calculate true U axis
                textureAxes[0].xyz = glm::normalize(
                    glm::cross(glm::vec3(textureAxes[1].xyz), plane.normal));

                // Now calculate the true V axis
                textureAxes[1].xyz = glm::normalize(
                    glm::cross(plane.normal, glm::vec3(textureAxes[0].xyz)));
            }
            else
            {
                textureAxes[0].xyz = Orientations::RightVectors[orientation];
            }
        }

        Plane plane{};

        Rc<Material> material;
        std::array<vec4, 2> textureAxes { vec4(0.0f), vec4(0.0f) };
        std::array<float, 2> scale { 1.0f, 1.0f };
        float rotate = 0;
        float lightmapScale = 16;
        uint32_t smoothing = 0;
        std::optional<DispInfo> disp;
    };

    struct Face
    {
        Face(Side* side, std::vector<vec3> points)
            : side(side)
            , points(std::move(points))
        {}

        Face(Face&& other) = default;
        Face(const Face& other) = default;
        Face& operator=(const Face& other) = default;

        Side* side;
        std::vector<vec3> points;
    };
}
