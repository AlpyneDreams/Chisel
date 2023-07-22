#pragma once

namespace chisel
{
    inline bool PointInsideConvex(const vec3& point, std::span<const vec3> vertices)
    {
        if (vertices.size() < 3)
            return false;

        glm::vec3 v0 = vertices[0];
        glm::vec3 v1 = vertices[1];
        glm::vec3 v2 = vertices[2];
        glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);

        for (size_t i = 0; i < vertices.size(); i++)
        {
            size_t j = (i + 1) % vertices.size();

            glm::vec3 vi = vertices[i];
            glm::vec3 vj = vertices[j];

            static constexpr float epsilon = 0.001f;
            if (glm::dot(normal, glm::cross(vj - vi, point - vi)) < -epsilon)
                return false;
        }

        return true;
    }
}
