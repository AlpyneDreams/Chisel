#include "chisel/map/Solid.h"
#include "chisel/Chisel.h"
#include "common/Bit.h"
#include "math/Winding.h"

#include <unordered_set>

namespace chisel
{
    ConVar<bool> r_disp_mask_solid("r_disp_mask_solid", true, "Hide unused faces of displacement brushes", [](bool& b) {
        for (auto& solid : Chisel.map) {
            if (solid.HasDisplacement())
                solid.UpdateMesh();
        }
    });

    inline void InitSideData(Side& side, Material *material)
    {
        side.material = material;
        side.rotate = 0.0f;

        side.textureAxes[0].w = 0.0f;
        side.textureAxes[1].w = 0.0f;

        side.scale[0] = 0.25f;
        side.scale[1] = 0.25f;

        side.textureAxes[0].xyz = vec3(0);
        side.textureAxes[1].xyz = vec3(0);

        Orientation orientation = Orientations::CalcOrientation(side.plane);
        if (orientation == Orientations::Invalid)
            return;

        side.textureAxes[1].xyz = Orientations::DownVectors[orientation];
        if (trans_texture_face_alignment)
        {
            // Calculate true U axis
            side.textureAxes[0].xyz = glm::normalize(
                glm::cross(glm::vec3(side.textureAxes[1].xyz), side.plane.normal));

            // Now calculate the true V axis
            side.textureAxes[1].xyz = glm::normalize(
                glm::cross(side.plane.normal, glm::vec3(side.textureAxes[0].xyz)));
        }
        else
        {
            side.textureAxes[0].xyz = Orientations::RightVectors[orientation];
        }
    }

    Solid::Solid(BrushEntity* parent)
        : Atom(parent)
    {
    }

    Solid::Solid(BrushEntity* parent, std::vector<Side> sides, bool initMesh)
        : Atom(parent)
        , m_sides(std::move(sides))
    {
        // Check if this brush has displacements
        for (Side& side : m_sides)
        {
            if (side.disp.has_value())
            {
                this->m_displacement = true;
                break;
            }
        }

        if (initMesh)
            UpdateMesh();
    }

    Solid::Solid(Solid&& other)
        : Atom(other.m_parent)
    {
        this->m_displacement = other.m_displacement;
        this->m_meshes = std::move(other.m_meshes);
        this->m_sides = std::move(other.m_sides);
        this->m_faces = std::move(other.m_faces);
        this->m_bounds = other.m_bounds;
    }
        
    Solid::~Solid()
    {
    }

    void Solid::UpdateMesh()
    {
        static bit::bitvector shouldUse;
        static std::unordered_set<AssetID> uniqueMaterials;

        BrushGPUAllocator& a = *Chisel.brushAllocator;

        // TODO: Avoid clearing meshes out every time.
        for (auto& mesh : m_meshes)
        {
            if (mesh.alloc)
            {
                a.free(*mesh.alloc);
                mesh.alloc = std::nullopt;
            }
        }

        uniqueMaterials.clear();
        uniqueMaterials.reserve(m_sides.size());
        m_meshes.clear();
        m_meshes.reserve(m_sides.size());
        m_faces.clear();
        m_faces.reserve(m_sides.size());
        shouldUse.clearAll();
        shouldUse.ensureSize(m_sides.size());

        for (uint32_t i = 0; i < m_sides.size(); i++)
        {
            // Displacements: exclude unused sides
            if (m_displacement && r_disp_mask_solid && !m_sides[i].disp.has_value())
                continue;

            AssetID id = InvalidAssetID;
            if (m_sides[i].material)
                id = m_sides[i].material->id;
            uniqueMaterials.insert(id);

            glm::vec3 normal0 = m_sides[i].plane.normal;
            float dist0 = m_sides[i].plane.Dist();
            if (normal0 == glm::vec3(0.0f))
            {
                shouldUse.set(i, false);
                continue;
            }

            shouldUse.set(i, true);
            for (uint32_t j = 0; j < i; j++)
            {
                glm::vec3 normal1 = m_sides[j].plane.normal;
                float dist1 = m_sides[j].plane.Dist();

                if (glm::dot(normal0, normal1) > 0.999f && (fabsf(dist0 - dist1) < 0.01f))
                {
                    shouldUse.set(j, false);
                    break;
                }
            }
        }

        // Convert from sides as planes to faces.
        for (uint32_t i = 0; i < shouldUse.dwordCount(); i++)
        {
            for (uint32_t idx : bit::BitMask(shouldUse.dword(i)))
            {
                uint32_t sideIdx = i * 32 + idx;

                Side& side = m_sides[sideIdx];

                Winding scratchWindings[2];
                auto* currentWinding = &scratchWindings[0];

                Winding::CreateFromPlane(side.plane, *currentWinding);
                for (uint32_t j = 0; j < m_sides.size() && currentWinding; j++)
                {
                    if (j != sideIdx)
                    {
                        Plane clipPlane = Plane(-m_sides[j].plane.normal, -m_sides[j].plane.offset);

                        currentWinding = Winding::Clip(clipPlane, *currentWinding, currentWinding == &scratchWindings[0] ? scratchWindings[1] : scratchWindings[0]);
                    }
                }

                if (currentWinding)
                {
                    // If a point in the winding is close enough to an integer coordinate,
                    // treat it as being at that coordinate.
                    // This matches Hammer's and VBSP's behaviour to combat imprecisions.
                    for (uint32_t j = 0; j < currentWinding->count; j++)
                    {
                        vec3& point = currentWinding->points[j];
                        for (uint32_t k = 0; k < 3; k++)
                        {
                            static constexpr float ROUND_VERTEX_EPSILON = 0.01f;
                            float val     = point[k];
                            float rounded = round(val);
                            if (math::CloseEnough(val, rounded, ROUND_VERTEX_EPSILON))
                                point[k] = rounded;
                        }
                    }

#if 0
                    // Remove duplicate points.
                    for (uint32_t i = 0; i < currentWinding->count; i++)
                    {
                        for (uint32_t j = i + 1; j < currentWinding->count; j++)
                        {
                            static constexpr float MIN_EDGE_LENGTH_EPSILON = 0.1f;
                            vec3 edge = currentWinding->points[i] - currentWinding->points[j];
                            if (glm::length(edge) < MIN_EDGE_LENGTH_EPSILON)
                            {
                                if (j + 1 < currentWinding->count)
                                {
                                    std::memmove(&(currentWinding->points[j]), &(currentWinding->points[j + 1]), (currentWinding->count - (j + 1)) * sizeof(currentWinding[0]));
                                    currentWinding->count = currentWinding->count - 1;
                                }
                            }
                        }
                    }
#endif
                    
                    m_faces.emplace_back(&side, std::vector<vec3>(currentWinding->points, currentWinding->points + currentWinding->count));
                }
            }
        }

        if (m_displacement)
            m_meshes.resize(m_faces.size());
        else
            m_meshes.resize(uniqueMaterials.size());

        m_bounds = std::nullopt;

        uint faceIdx = 0;

        static DispInfo dispDefault = DispInfo(0);

        // Create mesh from faces
        for (auto& face : m_faces)
        {
            if (m_displacement)
            {
                DispInfo& disp = face.side->disp.has_value() ? *(face.side->disp) : dispDefault;

                assert(face.points.size() >= 3);
                
                // 2^n x 2^m quads, 2 tris per quad, 3 verts per tri.
                uint numVertices = disp.verts.size();
                int length = disp.length;
                int quadLength = length - 1;
                uint numIndices = (2 * quadLength * quadLength) * 3;

                disp.UpdatePointStartIndex(face.points);

                vec3 edgeInt[2];
                edgeInt[0] = (face.points[(1 + disp.pointStartIndex) % 4] - face.points[(0 + disp.pointStartIndex) % 4]) / float(length - 1);
                edgeInt[1] = (face.points[(2 + disp.pointStartIndex) % 4] - face.points[(3 + disp.pointStartIndex) % 4]) / float(length - 1);

                auto& mesh = m_meshes[faceIdx];
                mesh.material = face.side->material;
                mesh.brush = this;
                mesh.vertices.reserve(numVertices);
                mesh.indices.reserve(numIndices);

                for (uint y = 0; y < length; y++)
                {
                    vec3 endPts[2];
                    endPts[0] = (edgeInt[0] * float(y)) + face.points[(0 + disp.pointStartIndex) % 4];
                    endPts[1] = (edgeInt[1] * float(y)) + face.points[(3 + disp.pointStartIndex) % 4];

                    vec3 seg = endPts[1] - endPts[0];
                    vec3 segInt = seg / float(length - 1);

                    for (uint x = 0; x < length; x++)
                    {
                        float xPercent = float(x) / float(quadLength);
                        float yPercent = float(y) / float(quadLength);

                        vec3 pos = endPts[0] + segInt * float(x);

                        DispVert& vert = disp[y][x];

                        // Add elevation if any
                        pos += face.side->plane.normal * disp.elevation;

                        // Apply subdivision surface offset (not typically used)
                        pos += vert.offset;

                        // Add displacement field direction (normal) scaled by distance
                        pos += vert.normal * vert.dist;

                        // Extend bounds
                        m_bounds = m_bounds
                            ? AABB::Extend(*m_bounds, pos)
                            : AABB { pos, pos };

                        // TODO: UVs
                        mesh.vertices.emplace_back(pos, face.side->plane.normal, glm::vec3(xPercent, yPercent, vert.alpha / 255.f));
                    }
                }

                for (uint y = 0; y < quadLength; y++)
                {
                    for (uint x = 0; x < quadLength; x++)
                    {
                        bool even = (y * length + x) % 2 == 0;
                        if (!even)
                        {
                            // 1, 2, 0 (clockwise from bottom left)
                            mesh.indices.push_back(y * length + x);
                            mesh.indices.push_back(y * length + x + 1);
                            mesh.indices.push_back((y + 1) * length + x);

                            // 3, 0, 2
                            mesh.indices.push_back((y + 1) * length + x + 1);
                            mesh.indices.push_back((y + 1) * length + x);
                            mesh.indices.push_back(y * length + x + 1);
                        }
                        else
                        {
                            // 1, 0, 3
                            mesh.indices.push_back((y + 1) * length + x + 1);
                            mesh.indices.push_back((y + 1) * length + x);
                            mesh.indices.push_back(y * length + x);

                            // 3, 2, 1
                            mesh.indices.push_back(y * length + x);
                            mesh.indices.push_back(y * length + x + 1);
                            mesh.indices.push_back((y + 1) * length + x + 1);
                        }
                    }
                }
            }
            else // regular brush
            {
                uint32_t numVertices = face.points.size();
                if (numVertices < 3)
                    continue;
                uint32_t numIndices = (numVertices - 2) * 3;

                AssetID id = InvalidAssetID;
                if (face.side->material)
                    id = face.side->material->id;

                uint32_t meshIdx = std::distance(uniqueMaterials.begin(), uniqueMaterials.find(id));
                auto& mesh = m_meshes[meshIdx];
                mesh.material = face.side->material;
                mesh.brush = this;
                uint32_t startingVertex = mesh.vertices.size();
                uint32_t startingIndex = mesh.indices.size();
                mesh.vertices.reserve(startingVertex + numVertices);
                mesh.indices.reserve(startingIndex + numIndices);
                for (uint32_t i = 0; i < numVertices; i++)
                {
                    vec3 pos = face.points[i];

                    float mappingWidth = 32.0f;
                    float mappingHeight = 32.0f;
                    if (face.side->material != nullptr && face.side->material->baseTexture != nullptr && face.side->material->baseTexture->texture != nullptr)
                    {
                        D3D11_TEXTURE2D_DESC desc;
                        face.side->material->baseTexture->texture->GetDesc(&desc);

                        mappingWidth = float(desc.Width);
                        mappingHeight = float(desc.Height);
                    }

                    float u = glm::dot(glm::vec3(face.side->textureAxes[0].xyz), glm::vec3(pos)) / face.side->scale[0] + face.side->textureAxes[0].w;
                    float v = glm::dot(glm::vec3(face.side->textureAxes[1].xyz), glm::vec3(pos)) / face.side->scale[1] + face.side->textureAxes[1].w;

                    u = mappingWidth ? u / float(mappingWidth) : 0.0f;
                    v = mappingHeight ? v / float(mappingHeight) : 0.0f;

                    mesh.vertices.emplace_back(pos, face.side->plane.normal, glm::vec3(u, v, 0.0f));
                    m_bounds = m_bounds
                        ? AABB::Extend(*m_bounds, pos)
                        : AABB{ pos, pos };
                }
                // Naiive fan-ing.
                // Should move to delaugney potentially.
                // Need to consider perf impact of that though compared to simple approach.
                const uint32_t numPolygons = numIndices / 3;
                for (uint32_t i = 0; i < numPolygons; i++)
                {
                    mesh.indices.emplace_back(startingVertex + i + 2);
                    mesh.indices.emplace_back(startingVertex + i + 1);
                    mesh.indices.emplace_back(startingVertex);
                }
            }
            faceIdx++;
        }

        // Upload all meshes after they're complete
        a.open();
        for (auto& mesh : m_meshes)
        {
            uint32_t verticesSize = sizeof(VertexSolid) * mesh.vertices.size();
            uint32_t indicesSize = sizeof(uint32_t) * mesh.indices.size();
            mesh.alloc = a.alloc(verticesSize + indicesSize);
            // Store vertices then indices.
            memcpy(&a.data()[mesh.alloc->offset + 0],            mesh.vertices.data(), verticesSize);
            memcpy(&a.data()[mesh.alloc->offset + verticesSize], mesh.indices.data(),  indicesSize);
        }
        a.close();
    }

    void Solid::Transform(const mat4x4& _matrix)
    {
        for (auto& side : m_sides)
            side.plane = side.plane.Transformed(_matrix);

        for (auto& side : m_sides)
        {
            mat4x4 trans = _matrix;

            bool locking = trans_texture_lock;
            bool scaleLocking = trans_texture_scale_lock;

            vec3 delta = trans[3].xyz;
            trans[3].xyz = glm::vec3(0.0f, 0.0f, 0.0f);
                
            bool moving = glm::length2(delta) > 0.00001f;

            if (trans == glm::identity<glm::mat4x4>())
            {
                if (moving && locking)
                {
                    side.textureAxes[0][3] -= glm::dot(delta, vec3(side.textureAxes[0].xyz)) / side.scale[0];
                    side.textureAxes[1][3] -= glm::dot(delta, vec3(side.textureAxes[1].xyz)) / side.scale[1];
                }

                continue;
            }

            vec3 u = side.textureAxes[0];
            vec3 v = side.textureAxes[1];

            float scaleU = glm::length(u);
            float scaleV = glm::length(v);
            if (scaleU <= 0.0f) scaleU = 1.0f;
            if (scaleV <= 0.0f) scaleV = 1.0f;

            u = glm::mat3(trans) * u;
            v = glm::mat3(trans) * v;

            scaleU = glm::length(u) / scaleU;
            scaleV = glm::length(v) / scaleV;
            if (scaleU <= 0.0f) scaleU = 1.0f;
            if (scaleV <= 0.0f) scaleV = 1.0f;

            bool uvAxisSameScale = math::CloseEnough(scaleU, 1.0f, 0.0001f) && math::CloseEnough(scaleV, 1.0f, 0.0001f);
            bool uvAxisPerpendicular = math::CloseEnough(glm::dot(u, v), 0.0f, 0.0025f);

            if (locking && uvAxisPerpendicular)
            {
                side.textureAxes[0].xyz = u / scaleU;
                side.textureAxes[1].xyz = v / scaleV;
            }

            if (uvAxisSameScale)
            {
                if (!locking)
                {
                    // TODO: re-init
                }
            }
            else
            {
                if (scaleLocking)
                {
                    side.scale[0] *= scaleU;
                    side.scale[1] *= scaleV;
                }
            }

            if (moving && locking)
            {
                side.textureAxes[0][3] -= glm::dot(delta, vec3(side.textureAxes[0].xyz)) / side.scale[0];
                side.textureAxes[1][3] -= glm::dot(delta, vec3(side.textureAxes[1].xyz)) / side.scale[1];
            }
        }

        UpdateMesh();
    }

    void Solid::AlignToGrid(vec3 gridSize)
    {
        auto bounds = GetBounds();
        if (!bounds)
            return;

        // Align this brush to mins of bounding box
        // like Hammer does (for when moving brushes)
        // on-grid.
        vec3 ref  = bounds->min;
        vec3 snap = math::Snap(bounds->min, gridSize);
        Transform(glm::translate(glm::identity<mat4x4>(), snap - ref));
    }

    Selectable* Solid::ResolveSelectable()
    {
        if (Chisel.selectMode == SelectMode::Solids)
            return this;

        // Groups/Objects
        if (m_parent->IsMap())
            return this;

        return m_parent;
    }

    bool Solid::IsSelected() const
    {
        return Atom::IsSelected() || m_parent->IsSelected();
    }

    void Solid::Delete()
    {
        m_parent->RemoveBrush(*this);
    }

    std::vector<Side> CreateCubeBrush(Material* material, vec3 size, const mat4x4& transform)
    {
        static const std::array<Plane, 6> kUnitCubePlanes =
        {
            Plane(vec3(+1,0,0), vec3(+1,0,0)),
            Plane(vec3(-1,0,0), vec3(-1,0,0)),
            Plane(vec3(0,+1,0), vec3(0,+1,0)),
            Plane(vec3(0,-1,0), vec3(0,-1,0)),
            Plane(vec3(0,0,+1), vec3(0,0,+1)),
            Plane(vec3(0,0,-1), vec3(0,0,-1))
        };

        std::vector<Side> sides;
        sides.resize(6);
        for (size_t i = 0; i < 6; i++)
        {
            sides[i].plane = kUnitCubePlanes[i].Transformed(glm::scale(transform, size));

            InitSideData(sides[i], material);
        }
        
        return sides;
    }

}
