#pragma once

#include "../CSG/Types.h"

#include "../CSG/FaceCache.h"
#include "../CSG/Side.h"
#include "../CSG/Userdata.h"
#include "../CSG/Volume.h"

namespace chisel::CSG
{
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

        const std::vector<Brush*>& GetIntersectingBrushes() const;

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
        friend glz::meta<Brush>;

        void MakeFaceCacheDirty();
        void MakeFragmentsDirty();

        CSGTree* m_tree;
        const ObjectID m_objectId;

        std::vector<Side> m_sides;
        std::optional<FaceCache> m_faceCache;

        std::vector<Brush*> m_intersectingBrushes;
        VolumeOperation m_volumeOperation = std::identity{};
        Order m_order = 0;
    };
}

template <>
struct glz::meta<chisel::CSG::Brush>
{
    using T = chisel::CSG::Brush;
    static constexpr auto value = glz::object(
        "object_id", &T::m_objectId,
        "order", &T::m_order,
        "sides", &T::m_sides
    );
};
