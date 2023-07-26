#pragma once

#include "common/Common.h"
#include "chisel/Selection.h"
#include "console/ConVar.h"
#include "render/Render.h"
#include "Types.h"
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

    struct Face : public Selectable
    {
        Face(Solid* brush, Side* side, std::vector<vec3> pts)
            : solid(brush)
            , side(side)
            , points(std::move(pts))
        {
            UpdateBounds();
        }

        Face(Face&& other) = default;
        Face(const Face& other) = default;
        Face& operator=(const Face& other) = default;

        Solid* solid;
        Side* side;
        std::vector<vec3> points;
        AABB bounds;
        uint meshIdx = 0;
        uint startIndex = 0;

        uint GetVertexCount() const { return points.size(); }
        uint GetIndexCount() const { return (GetVertexCount() - 2) * 3; }

        uint GetDispIndexCount() const
        {
            if (!side->disp)
                return GetIndexCount();
            return side->disp->GetIndexCount();
        }

        void UpdateBounds();

    // Selectable Interface //
        virtual std::optional<AABB> GetBounds() const override { return bounds; }
        virtual void Transform(const mat4x4& matrix) override;
        virtual void Delete() override {} // TODO
        virtual void AlignToGrid(vec3 gridSize) {} // TODO
        virtual Selectable* ResolveSelectable() { return this; } // TODO
        virtual Selectable* Duplicate() override { return nullptr; }
    };
}
