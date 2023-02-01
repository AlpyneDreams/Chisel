#pragma once

#include "../CSG/Types.h"

#include "../CSG/Brush.h"
#include "../CSG/Userdata.h"

namespace chisel::CSG
{
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
    protected:
        friend Brush;

        void MarkDirtyFaceCache(Brush& brush);
        void MarkDirtyFragments(Brush& brush);
    private:
        std::vector<Brush*> QueryIntersectingBrushes(const AABB& aabb, const Brush* ignore);
        ObjectID GetNewObjectID();

        std::list<Brush> m_brushes;
        std::unordered_set<Brush*> m_dirtyFaceCacheBrushes;
        std::unordered_set<Brush*> m_dirtyFragmentBrushes;
        VolumeID m_void = VolumeID(0);
        uint32_t m_lastObjectId = 0;
    };

}
