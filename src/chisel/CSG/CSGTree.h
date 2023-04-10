#pragma once

#include "../CSG/Types.h"

#include "../CSG/Brush.h"
#include "../CSG/Userdata.h"

namespace chisel::CSG
{
    struct RayHit
    {
        const Brush* brush;
        const Face* face;
        float t;
    };

    inline bool PointInsideConvex(const CSG::Vector3& point, const std::vector<Vertex>& vertices)
    {
        if (vertices.size() < 3)
            return false;

        glm::vec3 v0 = vertices[0].position;
        glm::vec3 v1 = vertices[1].position;
        glm::vec3 v2 = vertices[2].position;
        glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);

        for (size_t i = 0; i < vertices.size(); i++)
        {
            size_t j = (i + 1) % vertices.size();

            glm::vec3 vi = vertices[i].position;
            glm::vec3 vj = vertices[j].position;

            static constexpr float epsilon = 0.001f;
            if (glm::dot(normal, glm::cross(vj - vi, point - vi)) < -epsilon)
                return false;
        }

        return true;
    }

    class CSGTree : public UserdataProvider
    {
    public:
        CSGTree();
        ~CSGTree();

        Brush& CreateBrush();
        void DestroyBrush(Brush& brush);
        const std::list<Brush>& GetBrushes() const;

        void SetVoidVolume(VolumeID type);
        VolumeID GetVoidVolume() const;

        std::unordered_set<Brush*> Rebuild();

        std::optional<RayHit> QueryRay(const Ray& ray) const;
    protected:
        friend Brush;

        void MarkDirtyFaceCache(Brush& brush);
        void MarkDirtyFragments(Brush& brush);
    private:
        std::list<Brush*> QueryIntersectingBrushes(const AABB& aabb, const Brush* ignore);

        ObjectID GetNewObjectID();

        std::list<Brush> m_brushes;
        std::unordered_set<Brush*> m_dirtyFaceCacheBrushes;
        std::unordered_set<Brush*> m_dirtyFragmentBrushes;
        VolumeID m_void = VolumeID(0);
        uint32_t m_lastObjectId = 0;
    };

}
