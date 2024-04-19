#include "assets/Assets.h"
#include "core/IndexBuffer.h"
#include "core/Mesh.h"
#include "console/Console.h"
#include "core/VertexBuffer.h"
#include "core/VertexLayout.h"
#include "math/Math.h"
#include "chisel/map/Common.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

using namespace tinyobj;

namespace chisel
{
    // Standard source model scale from meters to hammer units (hu)
    // 1 hu = 1/16 ft, 1 ft = 30.48 cm, 1 m = 100 cm
    static constexpr float OBJ_MODEL_SCALE = float(100 * (16 / 30.48));

    AssetLoader<Mesh> OBJLoader = { ".OBJ", [](Mesh& mesh, const Buffer& file_data)
    {
        std::string string(&file_data.front(), &file_data.back() + 1);

        ObjReader obj;

        if (!obj.ParseFromString(string, "")) {
            Console.Error("[OBJ Loader] Failed to load '{}': {}", mesh.GetPath(), obj.Error());
            throw std::runtime_error(obj.Error());
        }

        if (!obj.Warning().empty()) {
            Console.Warning("[OBJ Loader] Warning while loading '{}': {}", mesh.GetPath(), obj.Warning());
        }

        auto& data      = obj.GetAttrib();
        auto& shapes    = obj.GetShapes();
        //auto& materials = obj.GetMaterials();

        for (auto& shape : shapes) {
            auto& group = mesh.AddGroup();

            // TODO: buffer ownership
            std::vector<VertexSolid>& verts = *new std::vector<VertexSolid>;
            std::vector<uint32>& indices = *new std::vector<uint32>;

            // Load all vertices
            for (size_t i = 0; i <= data.vertices.size() - 3; i += 3)
            {
                auto x = data.vertices[i + 0];
                auto y = data.vertices[i + 1];
                auto z = data.vertices[i + 2];
                verts.push_back(VertexSolid {
                    vec3(x, z, y) * OBJ_MODEL_SCALE, // Z-up
                    vec3(0, 0, 0),
                    vec3(0, 0, 0),
                    0
                });
            }

            // For each face
            for (auto indexOffset = 0; auto faceVerts : shape.mesh.num_face_vertices)
            {
                if (faceVerts != 3) {
                    Console.Warn("Non-triangular face in OBJ file!");
                }

                // For each vertex in face (swap winding order)
                for (auto v = faceVerts - 1; v >= 0; v--)
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

                        verts[idx].uv = vec3(tu, tv, 0);
                    }

                    // Colors
                    real_t r = data.colors[3 * index.vertex_index + 0];
                    real_t g = data.colors[3 * index.vertex_index + 1];
                    real_t b = data.colors[3 * index.vertex_index + 2];

                    //verts[idx].color = vec3(r, g, b);
                }
                indexOffset += faceVerts;
            }

            group.vertices = VertexBuffer(VertexSolid::Layout, &verts[0], verts.size() * sizeof(VertexSolid));
            group.indices = IndexBuffer(&indices[0], indices.size() * sizeof(uint32));
        }
    }};

}
