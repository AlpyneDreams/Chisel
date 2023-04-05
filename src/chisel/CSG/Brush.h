#pragma once

#include "../CSG/Types.h"

#include "../CSG/FaceCache.h"
#include "../CSG/Side.h"
#include "../CSG/Userdata.h"
#include "../CSG/Volume.h"

namespace chisel::CSG
{
    // Brush!
    // A convex solid made up of planes.
    class Brush : public UserdataProvider
    {
    public:
        Brush(CSGTree* tree, ObjectID id);
        ~Brush();

        CSGTree* GetTree() const { return m_tree; }

        // TODO: Use Span type.
        void SetSides(const Side* first, const Side* end);
        const std::vector<Side>& GetSides() const;

        void SetOrder(Order order);
        Order GetOrder() const;

        void SetVolumeOperation(VolumeOperation op);
        template <typename... Args>
        VolumeID PerformVolumeOperation(Args&... args) const
        {
            return m_volumeOperation(std::forward<Args>(args)...);
        }

        ObjectID GetObjectID() const;
        std::optional<AABB> GetBounds() const;

        const std::list<Brush*>& GetIntersectingBrushes() const;

        bool ComesBefore(const Brush& other) const;

        Face* GetFace(size_t index);
        const std::vector<Face>& GetFaces() const;

        void Transform(const Matrix4& matrix);
        void AlignToGrid(const Vector3& gridSize);
    protected:
        friend class CSGTree;

        void RebuildFaceCache();
        void RebuildIntersectingBrushes();

        void RebuildFaceFragments();
    private:
        void MakeFaceCacheDirty();
        void MakeFragmentsDirty();

        CSGTree* m_tree;
        const ObjectID m_objectId;

        std::vector<Side> m_sides;
        std::optional<FaceCache> m_faceCache;

        std::list<Brush*> m_intersectingBrushes;
        VolumeOperation m_volumeOperation = std::identity{};
        Order m_order = 0;
    };
}
