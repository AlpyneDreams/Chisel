#include "../CSG/Face.h"
#include "../CSG/Edge.h"
#include "../CSG/Fragment.h"

#include "../CSG/CSGTree.h"

namespace chisel::CSG
{
    FaceVertexRelation Face::CalculateVertexRelation(const Vertex& vertex) const
    {
        // Check the relation based on the signed distance
        // of the point to the face plane.
        // If they are equal, then they are aligned,
        // otherwise we are on either side of the plane.
        float dist = side->plane.SignedDistance(vertex.position);
        if (CloseEnough(dist, 0.0))
            return FaceVertexRelations::Aligned;
        else if (dist > 0.0)
            return FaceVertexRelations::Front;
        else
            return FaceVertexRelations::Back;
    }

    void Face::OrderVertices()
    {
        // Use the info we have on the vertices (the planes that meet in the vertex)
        // to order the vertices properly without any floating point calculations.
        std::vector<Vertex> unordered = std::move(vertices);
        vertices.clear();

        auto curr = unordered.begin();
        while (curr != unordered.end())
        {
            Vertex v = *curr;
            vertices.push_back(v);
            unordered.erase(curr);
            curr = std::find_if(unordered.begin(), unordered.end(), [v](const Vertex& other)
            {
                // Does an edge exist between v and the other?
                return !!Vertex::FindEdge(v, other);
            });
        }
    }

    void Face::FixWinding()
    {
        // Just reverse the order of vertices if the polygon normal doesn't
        // match the plane normal.
        if (vertices.size() < 3)
            return;

        float d = dot(cross(
            vertices[1].position - vertices[0].position,
            vertices[2].position - vertices[0].position), side->plane.normal);

        if (d < 0)
            std::reverse(vertices.begin(), vertices.end());
    }

//-------------------------------------------------------------------------------------------------

    std::optional<Vertex> Face::CreateVertex(std::array<Face*, 3> faces)
    {
        std::sort(faces.begin(), faces.end());

        Matrix3 m;
        m = glm::row(m, 0, faces[0]->side->plane.normal);
        m = glm::row(m, 1, faces[1]->side->plane.normal);
        m = glm::row(m, 2, faces[2]->side->plane.normal);

        Unit det = glm::determinant(m);
        if (CloseEnough(det, 0.0))
            return std::nullopt;

        Matrix3 mx(m), my(m), mz(m);
        Vector3 offsets{ faces[0]->side->plane.offset, faces[1]->side->plane.offset, faces[2]->side->plane.offset };
        mx = glm::column(mx, 0, -offsets);
        my = glm::column(my, 1, -offsets);
        mz = glm::column(mz, 2, -offsets);

        Vector3 position = Vector3
        {
            glm::determinant(mx) / det,
            glm::determinant(my) / det,
            glm::determinant(mz) / det,
        };

        return Vertex{ faces, position };
    }

//-------------------------------------------------------------------------------------------------

    static std::pair<Fragment, Fragment> Split(const Fragment& fragment, Face* splitter)
    {
        // Splits fragment into front and back piece w.r.t. face.
        // Call only if test(fragment, face) == RELATION_SPLIT

        // Front + Back.
        std::array<Fragment, FragmentFaceRelations::Count> pieces =
        {
            // Front
            Fragment
            {
                .face  = fragment.face,
                .front = fragment.front,
                .back  = fragment.back,
                // Vertices + relation is purposefully not set here.
            },
            // Back
            Fragment
            {
                .face  = fragment.face,
                .front = fragment.front,
                .back  = fragment.back,
                // Vertices + relation is purposefully not set here.
            },
            // Aligned (dummy)
            Fragment {},
        };

        const size_t vertexCount = fragment.vertices.size();
        for (size_t i = 0; i < vertexCount; i++)
        {
            const size_t j = (i + 1) % vertexCount;

            const Vertex& v0 = fragment.vertices[i];
            const Vertex& v1 = fragment.vertices[j];

            const FaceVertexRelation c0 = splitter->CalculateVertexRelation(v0);
            const FaceVertexRelation c1 = splitter->CalculateVertexRelation(v1);
            if (c0 != c1)
            {
                auto edge = Vertex::FindEdge(v0, v1);
                if (!edge)
                {
                    // This shouldn't happen.
                    fprintf(stderr, "This shouldn't happen (A).\n");
                    pieces[c0].vertices.push_back(v0);
                    continue;
                }

                std::array<Face*, 3> trio{{ edge->faces[0], edge->faces[1], splitter }};
                auto v = Face::CreateVertex(trio);
                if (!v)
                {
                    // This shouldn't happen.
                    fprintf(stderr, "This shouldn't happen (B).\n");
                    pieces[c0].vertices.push_back(v0);
                    continue;
                }

                if (c0 == FaceVertexRelations::Aligned)
                {
                    pieces[c1].vertices.push_back(*v);
                }
                else if (c1 == FaceVertexRelations::Aligned)
                {
                    pieces[c0].vertices.push_back(v0);
                    pieces[c0].vertices.push_back(*v);
                }
                else
                {
                    pieces[c0].vertices.push_back(v0);
                    pieces[c0].vertices.push_back(*v);
                    pieces[c1].vertices.push_back(*v);
                }
            }
            else
            {
                pieces[c0].vertices.push_back(v0);
            }
        }

        return std::make_pair(std::move(pieces[0]), std::move(pieces[1]));
    }

    static std::vector<Fragment> Carve(Fragment fragment, Brush* brush, size_t faceIndex = 0)
    {
        // This carves the fragment into pieces that can be uniquely
        // classified as being inside/outside/aligned/reverse aligned
        // with the given brush. It does this by pushing the fragment
        // down the convex bsp-tree (the list of faces) of the given brush.
        Face* face = brush->GetFace(faceIndex);
        if (!face)
            return { std::move(fragment) };

        switch (auto relation = fragment.CalculateFaceRelation(*face))
        {
            case FragmentFaceRelations::Outside: // In Front
            {
                fragment.relation = relation;
                return { std::move(fragment) };
            }
            case FragmentFaceRelations::Aligned:
            case FragmentFaceRelations::ReverseAligned:
            {
                fragment.relation = relation;
            }
            [[fallthrough]];
            case FragmentFaceRelations::Inside:
            {
                // Push this fragment down the BSP tree.
                return Carve(std::move(fragment), brush, faceIndex + 1);
            }
            case FragmentFaceRelations::Split:
            {
                auto [front, back] = Split(fragment, face);

                // Push the back fragment further down the BSP tree.
                back.relation = fragment.relation;
                auto rest = Carve(std::move(back), brush, faceIndex + 1);

                // Prevent redundant splitting.
                if (rest.size() == 1 && rest[0].relation == FragmentFaceRelations::Outside)
                {
                    fragment.relation = FragmentFaceRelations::Outside;
                    return { std::move(fragment) };
                }

                front.relation = FragmentFaceRelations::Outside;
                rest.push_back(std::move(front));
                return rest;
            }
            default:
            {
                // Should never get here.
                fprintf(stderr, "Oh no.\n");
                return {};
            };
        }
    }

    void Face::RebuildFragments(VolumeID voidVolume, const Brush& brush)
    {
        fragments.clear();

        // Add the first fragment.
        fragments.emplace_back(Fragment
        {
            .face = this,
            .front =
            {
                .volume = voidVolume,
                .brush  = nullptr,
            },
            .back =
            {
                .volume = brush.PerformVolumeOperation(voidVolume),
                .brush  = &brush,
            },
            .vertices = vertices,
        });

        // Use each intersecting brush to carve this brush's fragments
        // into pieces, then depending on the piece's relation to the intersecting
        // brush (inside/outside/aligned/reverse aligned) and the order between this
        // brush and the intersecting brush, adjust the piece's front/back volumes
        // or discard the piece.
        for (Brush* intersecting : brush.GetIntersectingBrushes())
        {
            const bool beforeIntersecting = brush.ComesBefore(*intersecting);
            const std::ptrdiff_t count = fragments.size();

            for (std::ptrdiff_t index = count - 1; index >= 0; index--)
            {
                Fragment fragment = std::move(fragments[index]);
                fragments.erase(fragments.begin() + index);

                fragment.relation = FragmentFaceRelations::Inside;
                std::vector<Fragment> pieces = Carve(std::move(fragment), intersecting);
                for (auto& piece : pieces)
                {
                    bool keepPiece = true;

                    assert(piece.relation.has_value());
                    switch (*piece.relation)
                    {
                        case FragmentFaceRelations::Inside:
                        {
                            if (beforeIntersecting)
                            {
                                piece.back = FragmentDirData
                                {
                                    .volume = intersecting->PerformVolumeOperation(piece.back.volume),
                                    .brush  = intersecting,
                                };
                            }
                            piece.front = FragmentDirData
                            {
                                .volume = intersecting->PerformVolumeOperation(piece.front.volume),
                                .brush  = intersecting,
                            };
                            break;
                        }
                        case FragmentFaceRelations::Aligned:
                        {
                            if (beforeIntersecting)
                                keepPiece = false;
                            break;
                        }
                        case FragmentFaceRelations::ReverseAligned:
                        {
                            if (beforeIntersecting)
                            {
                                keepPiece = false;
                            }
                            else
                            {
                                piece.front = FragmentDirData
                                {
                                    .volume = intersecting->PerformVolumeOperation(piece.front.volume),
                                    .brush  = intersecting,
                                };
                            }
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }

                    if (keepPiece)
                    {
                        fragments.emplace_back(std::move(piece));
                    }
                }
            }
        }
    }

}
