#pragma once

#include "math/Math.h"
#include "math/Ray.h"
#include "math/Plane.h"
#include "core/Transform.h"

#include "platform/Window.h"
#include "render/Render.h"

namespace chisel
{
    struct Camera
    {
        // Horizontal field of view in degrees
        float fieldOfView = 90.0f;

        // Reduce or expand FOV based on aspect ratio.
        bool scaleFOV = false;
        // Base aspect ratio for FOV scaling.
        vec2 scaleFOVAspect = vec2(16, 9);

        float near = 0.1f;
        float far  = 16384.f;

        // If not null then camera only renders to this texture
        render::RenderTarget* renderTarget;

        // TODO: Move all of this stuff to Transform

        // TODO: Less cos/sin. The direction vectors can be computed faster by
        // multiplying a quat by a unit vector, or from a matrix column.

        vec3 position;
        vec3 angles;

        bool rightHanded = true;

        vec3 Up() const
        {
            glm::mat4 roll_mat = glm::rotate(glm::mat4(1.0f), angles.z, Forward());
            return glm::normalize(glm::mat3(roll_mat) * Vectors.Up);
        }

        vec3 Right() const
        {
            vec3 res = glm::normalize(glm::cross(Forward(), Vectors.Up));
            return rightHanded ? res : -res;
        }

        vec3 Forward() const
        {
            // TODO: Genericize the Forward function
            // to account for other Ups.
            const float x = std::cos(angles.x) * std::cos(angles.y);
            const float y = std::sin(angles.x);
            const float z = std::cos(angles.x) * std::sin(angles.y);

            return glm::normalize(vec3(x, z, y));
        }


        // TODO: Store viewport rect in camera?
        Ray ScreenPointToRay(vec2 pos, Rect viewport)
        {
            vec2 rayNDC     = vec2(pos) / vec2(viewport.size * 0.5f) - 1.0f;
            vec4 rayClip    = vec4(rayNDC.x, -rayNDC.y, -1, 1);
            vec4 rayView    = glm::inverse(ProjMatrix()) * rayClip;
            vec4 rayWorld   = glm::inverse(ViewMatrix()) * vec4(rayView.xyz, 0);
            return Ray(position, rayWorld.xyz);
        }

    public:
        // World to camera matrix
        mat4x4 ViewMatrix()
        {
            if (overrideViewMatrix)
                return view;

            if (rightHanded)
            {
                view = glm::lookAtRH(
                    position,
                    position + Forward(),
                    Up());
            }
            else
            {
                view = glm::lookAtLH(
                    position,
                    position + Forward(),
                    Up());
            }

            return view;
        }

        // Set custom world to camera matrix
        void SetViewMatrix(mat4x4& m)
        {
            overrideViewMatrix = true;
            view = m;
        }

        void ResetViewMatrix() {
            overrideViewMatrix = false;
        }

    public:
        float AspectRatio()
        {
            vec2 size;
            if (renderTarget) {
                size = renderTarget->GetSize();
            } else {
                size = Window::main->GetSize();
            }

            return size.x / size.y;
        }

        void GetFOV(float& fovX, float& fovY)
        {
            fovX = fieldOfView;
            if (scaleFOV)
                fovX = ScaleFOVByWidthRatio(fovX, aspectRatio, (scaleFOVAspect.x / scaleFOVAspect.y));
            fovY = CalcVerticalFOV(fovX, aspectRatio);
        }

        // Projection matrix
        mat4x4 ProjMatrix()
        {
            if (overrideProjMatrix)
                return proj;

            aspectRatio = AspectRatio();

            float fovX, fovY;
            GetFOV(fovX, fovY);

            if (rightHanded)
                proj = glm::perspectiveRH_ZO(math::radians(fovY), aspectRatio, near, far);
            else
                proj = glm::perspectiveLH_ZO(math::radians(fovY), aspectRatio, near, far);

            return proj;
        }

        // Set custom projection matrix
        void SetProjMatrix(mat4x4& m)
        {
            overrideProjMatrix = true;
            view = m;
        }

        void ResetProjMatrix() {
            overrideProjMatrix = false;
        }

        Frustum CreateFrustum()
        {
            float fovX, fovY;
            GetFOV(fovX, fovY);
            const glm::vec3 frontMultFar = far * Forward();

            const float halfVSide = far * tanf(fovY * .5f);
            const float halfHSide = halfVSide * aspectRatio;

            return Frustum
            {
                .topFace    = { position, glm::cross(Right(), frontMultFar - Up() * halfVSide) },
                .bottomFace = { position, glm::cross(frontMultFar + Up() * halfVSide, Right()) },
                .rightFace  = { position, glm::cross(frontMultFar - Right() * halfHSide, Up()) },
                .leftFace   = { position, glm::cross(Up(),frontMultFar + Right() * halfHSide) },
                .farFace    = { position + frontMultFar, -Forward() },
                .nearFace   = { position + near * Forward(), Forward() },
            };
        }

    private:
        // Aspect ratio W/H
        float aspectRatio = 16.0f / 9.0f;

        // World to camera matrix
        mat4x4 view;

        // Projection matrix
        mat4x4 proj;

        bool overrideViewMatrix = false;
        bool overrideProjMatrix = false;

    public:
        static float ScaleFOV(float fovDegrees, float ratio) {
            return math::degrees(atan(tan(glm::radians(fovDegrees) * 0.5f) * ratio)) * 2.0f;
        }

        // Calculate vertical Y-FOV from X-FOV based on W/H aspect ratio
        static float CalcVerticalFOV(float fovx, float aspectRatio) {
            return ScaleFOV(fovx, 1.f/aspectRatio);
        }

        // Calculate horizontal X-FOV from Y-FOV based on W/H aspect ratio
        static float CalcHorizontalFOV(float fovy, float aspectRatio) {
            return ScaleFOV(fovy, aspectRatio);
        }

        // Scale (reduce or expand) FOV based on ratio of two aspect ratios
        // Typically the base aspect is a reference aspect like 16:9 or 4:3
        static float ScaleFOVByWidthRatio(float fov, float aspect, float baseAspect) {
            return ScaleFOV(fov, aspect / baseAspect);
        }
    };
}