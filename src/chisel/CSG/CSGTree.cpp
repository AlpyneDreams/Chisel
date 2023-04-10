#include "../CSG/CSGTree.h"

namespace chisel::CSG
{
    CSGTree::CSGTree()
    {
    }

    CSGTree::~CSGTree()
    {
        m_brushes.clear();
    }

//-------------------------------------------------------------------------------------------------

    Brush& CSGTree::CreateBrush()
    {
        return m_brushes.emplace_back(this, GetNewObjectID());
    }

    void CSGTree::DestroyBrush(Brush& brush)
    {
        for (Brush* neighbor : brush.m_intersectingBrushes)
            neighbor->m_intersectingBrushes.remove(&brush);
        std::erase_if(m_dirtyFaceCacheBrushes, [ptr = &brush](Brush* val){ return val == ptr ; });
        std::erase_if(m_dirtyFragmentBrushes, [ptr = &brush](Brush* val){ return val == ptr ; });
        m_brushes.remove_if([ptr = &brush](Brush& val){ return &val == ptr ; });
    }

    const std::list<Brush>& CSGTree::GetBrushes() const
    {
        return m_brushes;
    }


    void CSGTree::SetVoidVolume(VolumeID type)
    {
        if (m_void == type)
            return;

        m_void = type;
        for (auto& brush : m_brushes)
            m_dirtyFragmentBrushes.insert(&brush);
    }

    VolumeID CSGTree::GetVoidVolume() const
    {
        return m_void;
    }


    std::unordered_set<Brush*>CSGTree::Rebuild()
    {
        for (auto* brush : m_dirtyFaceCacheBrushes)
        {
            brush->RebuildFaceCache();
            m_dirtyFragmentBrushes.insert(brush);
        }

        for (auto* brush : m_dirtyFaceCacheBrushes)
        {
            brush->RebuildIntersectingBrushes();
            for (auto* intersecting : brush->m_intersectingBrushes)
                m_dirtyFragmentBrushes.insert(intersecting);
        }

        for (auto* brush: m_dirtyFragmentBrushes)
        {
            if (!m_dirtyFaceCacheBrushes.contains(brush))
                brush->RebuildIntersectingBrushes();
            brush->RebuildFaceFragments();
        }

        std::unordered_set<Brush*> rebuilt = std::move(m_dirtyFragmentBrushes);
        m_dirtyFaceCacheBrushes.clear();
        m_dirtyFragmentBrushes.clear();
        return rebuilt;
    }

//-------------------------------------------------------------------------------------------------

    void CSGTree::MarkDirtyFaceCache(Brush& brush)
    {
        m_dirtyFaceCacheBrushes.insert(&brush);
    }

    void CSGTree::MarkDirtyFragments(Brush& brush)
    {
        m_dirtyFragmentBrushes.insert(&brush);
    }

//-------------------------------------------------------------------------------------------------

    std::list<Brush*> CSGTree::QueryIntersectingBrushes(const AABB& aabb, const Brush* ignore = nullptr)
    {
        std::list<Brush*> result;
        for (auto& brush : m_brushes)
        {
            if (!brush.GetBounds())
                continue;

            if (ignore && &brush == ignore)
                continue;

            if (brush.GetBounds()->Intersects(aabb))
                result.push_back(&brush);
        }
        return result;
    }

    std::optional<RayHit> CSGTree::QueryRay(const Ray& ray) const
    {
        std::optional<RayHit> hit;

        for (const auto& brush : m_brushes)
        {
            auto bounds = brush.GetBounds();
            if (!bounds)
                continue;

            if (!ray.Intersects(*bounds))
                continue;

            for (const auto& face : brush.GetFaces())
            {
                float t;
                if (!ray.Intersects(face.side->plane, t))
                    continue;

                CSG::Vector3 intersection = ray.GetPoint(t);
                if (!PointInsideConvex(intersection, face.vertices))
                    continue;

                RayHit thisHit =
                {
                    .brush    = &brush,
                    .face     = &face,
                    .t        = t,
                };

                if (!hit || t < hit->t)
                    hit = thisHit;
            }
        }

        return hit;
    }

    ObjectID CSGTree::GetNewObjectID()
    {
        return ++m_lastObjectId;
    }

}
