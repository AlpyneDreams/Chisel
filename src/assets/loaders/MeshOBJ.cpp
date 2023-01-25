#include "assets/Assets.h"
#include "core/IndexBuffer.h"
#include "core/Mesh.h"
#include "console/Console.h"
#include "core/VertexBuffer.h"
#include "core/VertexLayout.h"
#include "math/Math.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

using namespace tinyobj;

namespace chisel
{
    struct VertexOBJ
    {
        vec3 position;
        vec3 normal;
        vec2 texcoord;
        vec3 color;

    };

    template <>
    Mesh* ImportAsset<Mesh, FixedString(".OBJ")>(const fs::Path& path)
    {
        static VertexLayout LayoutOBJ = VertexLayout {
            VertexAttribute::For<float>(3, VertexAttribute::Position),
            VertexAttribute::For<float>(3, VertexAttribute::Normal, true),
            VertexAttribute::For<float>(2, VertexAttribute::TexCoord),
            VertexAttribute::For<float>(3, VertexAttribute::Color),
        };

        ObjReader obj;

        if (!obj.ParseFromFile(path.string())) {
            if (!obj.Error().empty()) {
                Console.Error("[OBJ Loader] Failed to load '{}': {}", path, obj.Error());
            }
            return new Mesh();
        }

        if (!obj.Warning().empty()) {
            Console.Warning("[OBJ Loader] Warning while loading '{}': {}", path, obj.Warning());
        }

        Mesh* mesh = new Mesh();

        auto& data      = obj.GetAttrib();
        auto& shapes    = obj.GetShapes();
        //auto& materials = obj.GetMaterials();

        for (auto& shape : shapes) {
            auto& group = mesh->AddGroup();

            // TODO: buffer ownership
            std::vector<VertexOBJ>& verts = *new std::vector<VertexOBJ>;
            std::vector<uint32>& indices = *new std::vector<uint32>;

            // Load all vertices
            for (size_t i = 0; i <= data.vertices.size() - 3; i += 3)
            {
                auto x = data.vertices[i + 0];
                auto y = data.vertices[i + 1];
                auto z = data.vertices[i + 2];
                verts.push_back(VertexOBJ {
                    vec3(x, y, z),
                    vec3(0, 0, 0),
                    vec2(0, 0),
                    vec3(1, 0, 0)
                });
            }

            // For each face
            for (auto indexOffset = 0; auto faceVerts : shape.mesh.num_face_vertices)
            {
                if (faceVerts != 3) {
                    Console.Warn("Non-triangular face in OBJ file!");
                }

                // For each vertex in face
                for (auto v = 0; v < faceVerts; v++)
                {
                    index_t index = shape.mesh.indices[indexOffset + v];
                    size_t idx = index.vertex_index;

                    indices.push_back(idx);

                    //real_t vx = data.vertices[3 * index.vertex_index + 0];
                    //real_t vy = data.vertices[3 * index.vertex_index + 1];
                    //real_t vz = data.vertices[3 * index.vertex_index + 2];

                    // Normals
                    if (index.normal_index >= 0) {
                        real_t nx = data.normals[3 * index.normal_index + 0];
                        real_t ny = data.normals[3 * index.normal_index + 1];
                        real_t nz = data.normals[3 * index.normal_index + 2];

                        verts[idx].normal = vec3(nx, ny, nz);
                    }

                    // Texcoords
                    if (index.texcoord_index >= 0) {
                        real_t tu = data.texcoords[2 * index.texcoord_index + 0];
                        real_t tv = data.texcoords[2 * index.texcoord_index + 1];

                        verts[idx].texcoord = vec2(tu, tv);
                    }

                    // Colors
                    real_t r = data.colors[3 * index.vertex_index + 0];
                    real_t g = data.colors[3 * index.vertex_index + 1];
                    real_t b = data.colors[3 * index.vertex_index + 2];

                    verts[idx].color = vec3(r, g, b);
                }
                indexOffset += faceVerts;
            }

            group.vertices = VertexBuffer(LayoutOBJ, &verts[0], verts.size() * sizeof(VertexOBJ));
            group.indices = IndexBuffer(&indices[0], indices.size() * sizeof(uint32));
        }

        return mesh;
    }

}
