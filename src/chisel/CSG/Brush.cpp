#include "../CSG/Brush.h"
#include "../CSG/FaceCache.h"
#include "../CSG/CSGTree.h"

namespace chisel::CSG
{
    static bool SortFunc(const Brush* a, const Brush* b)
    {
        if (a->GetOrder() == b->GetOrder())
            return a->GetObjectID() < b->GetObjectID();
        return a->GetOrder() < b->GetOrder();
    }

//-------------------------------------------------------------------------------------------------

    Brush::Brush(CSGTree* tree, ObjectID id)
        : m_tree(tree)
        , m_objectId(id)
    {
    }

    Brush::~Brush()
    {
        for (auto* brush : m_intersectingBrushes)
            m_tree->MarkDirtyFragments(*brush);
    }

//-------------------------------------------------------------------------------------------------

    void Brush::SetSides(const Side* first, const Side* end)
    {
        m_sides = std::vector(first, end);
        MakeFaceCacheDirty();
    }

    const std::vector<Side>& Brush::GetSides() const
    {
        return m_sides;
    }

    void Brush::SetOrder(Order order)
    {
        if (m_order == order)
            return;

        m_order = order;
        MakeFragmentsDirty();
    }

    Order Brush::GetOrder() const
    {
        return m_order;
    }

    void Brush::SetVolumeOperation(VolumeOperation op)
    {
        m_volumeOperation = op;
        MakeFragmentsDirty();
    }

    ObjectID Brush::GetObjectID() const
    {
        return m_objectId;
    }

    std::optional<AABB> Brush::GetBounds() const
    {
        if (!m_faceCache)
            return std::nullopt;

        return m_faceCache->GetBounds();
    }

    const std::vector<Brush*>& Brush::GetIntersectingBrushes() const
    {
        return m_intersectingBrushes;
    }

    bool Brush::ComesBefore(const Brush& other) const
    {
        return SortFunc(this, &other);
    }

    Face* Brush::GetFace(size_t index)
    {
        if (!m_faceCache)
            return nullptr;

        auto& faces = m_faceCache->GetFaces();
        if (index >= faces.size())
            return nullptr;

        return &faces[index];
    }

    static const std::vector<Face> kEmptyFaceSet{};
    const std::vector<Face>& Brush::GetFaces() const
    {
        if (!m_faceCache)
            return kEmptyFaceSet;

        return m_faceCache->GetFaces();
    }

    void Brush::Transform(const Matrix4& matrix)
    {
        for (auto& side : m_sides)
            side.plane = side.plane.Transformed(matrix);
        MakeFaceCacheDirty();
    }

    void Brush::AlignToGrid(const Vector3& gridSize)
    {
        auto bounds = GetBounds();
        if (!bounds)
            return;

        // Align this brush to mins of bounding box
        // like Hammer does (for when moving brushes)
        // on-grid.
        CSG::Vector3 ref  = bounds->min;
        CSG::Vector3 snap = Snap(bounds->min, gridSize);
        Transform(glm::translate(glm::identity<CSG::Matrix4>(), snap - ref));
    }

//-------------------------------------------------------------------------------------------------

    void Brush::RebuildFaceCache()
    {
        m_faceCache = FaceCache(m_sides);
    }

    void Brush::RebuildIntersectingBrushes()
    {
        m_intersectingBrushes.clear();

        auto bounds = GetBounds();
        if (!bounds)
            return;

        m_intersectingBrushes = m_tree->QueryIntersectingBrushes(*bounds, this);
        // Sort so we always split in the same way.
        std::sort(m_intersectingBrushes.begin(), m_intersectingBrushes.end(), SortFunc);
    }

//-------------------------------------------------------------------------------------------------

    void Brush::RebuildFaceFragments()
    {
        if (!m_faceCache)
            return;

        for (auto& face : m_faceCache->GetFaces())
            face.RebuildFragments(m_tree->GetVoidVolume(), *this);
    }

//-------------------------------------------------------------------------------------------------

    void Brush::MakeFaceCacheDirty()
    {
        m_tree->MarkDirtyFaceCache(*this);
        MakeFragmentsDirty();
    }

    void Brush::MakeFragmentsDirty()
    {
        m_tree->MarkDirtyFragments(*this);
        for (auto* brush : m_intersectingBrushes)
            m_tree->MarkDirtyFragments(*brush);
    }

}
