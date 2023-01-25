#pragma once

#include "math/Math.h"

namespace engine
{
    struct Transform
    {
        vec3 position;
        quat rotation;
        vec3 scale = vec3(1, 1, 1);

        Transform(vec3 pos = Vectors.Zero, quat rot = quat(), vec3 scale = Vectors.One)
            : position(pos), rotation(rot), scale(scale) {}
        
        // Normalized forward (+Z) direction vector
        vec3 Forward() const {
            return rotation * Vectors.Forward;
        }

        // Normalized up (+Y) direction vector
        vec3 Up() const {
            return rotation * Vectors.Up;
        }
        
        // Normalized right (+X) direction vector
        vec3 Right() const {
            return rotation * Vectors.Right;
        }

        mat4x4 GetTransformMatrix() const
        {
            mat4x4 trs = glm::translate(glm::mat4x4(1), position);
            trs *= glm::mat4x4(rotation);
            trs = glm::scale(trs, scale);
            return trs;
        }
        
        void SetEulerAngles(vec3 degrees)
        {
            rotation = quat(glm::radians(degrees));
        #if defined(EDITOR)
            // Cache Euler angles. This avoids weird behavior when performing
            // round trip conversions from Euler -> Quaternion -> Euler in
            // the inspector.
            eulerAngles = degrees;
            eulerAnglesRotation = rotation;
        #endif
        }

        vec3 GetEulerAngles()
        {
        #if defined(EDITOR)
            // Return cached euler angles in edit mode
            // as long as the current rotation has not changed.
            if (rotation == eulerAnglesRotation) [[likely]]
                return eulerAngles;
            
            // If eulerAngles are not cached then recalculate them
            eulerAnglesRotation = rotation;
            eulerAngles = glm::degrees(glm::eulerAngles(rotation));
            return eulerAngles;
        #endif
            // TODO: Normalize quaternion?
            return glm::degrees(glm::eulerAngles(rotation));
        }

    private:
    #if defined(EDITOR)
        // Cached euler angles in degrees
        vec3 eulerAngles;

        // Rotation from cached eulerAngles
        quat eulerAnglesRotation;
    #endif

    };
}