#include "assets/Assets.h"
#include "core/Mesh.h"
#include "math/Math.h"
#include "chisel/map/Common.h"

#include <sstream>

#define LIBMDL_CUSTOM_VECTORS
namespace libmdl
{
    using vec2 = chisel::vec2;
    using vec3 = chisel::vec3;
    using vec4 = chisel::vec4;
}

#include "libmdl-plusplus/libmdl++.hpp"

namespace mdl = libmdl::mdl;
namespace vtx = libmdl::vtx;
namespace vvd = libmdl::vvd;

namespace chisel
{
    static MultiFileAssetLoader<Mesh> MDLLoader = { {".MDL", ".VVD", ".DX90.VTX"}, [](Mesh& outMesh, const std::span<Buffer>& buffers)
    {
        if (buffers.size() != 3)
            throw std::runtime_error("MDL Loader requires 3 files: .mdl, .vvd, .dx90.vtx");
        if (!buffers[0].size())
            throw std::runtime_error("MDL Loader requires .mdl file");
        if (!buffers[1].size())
            throw std::runtime_error("MDL Loader requires .vvd file");
        if (!buffers[2].size())
            throw std::runtime_error("MDL Loader requires .dx90.vtx file");

        //AssetLoader<Mesh>::ForExtension(".OBJ")->Load(mesh, fs::Path("models/teapot.obj")); // Test
        //return;

        libmdl::ModelData model(buffers[0], buffers[1], buffers[2]);
        const libmdl::MDLHeader& mdlData = model.getMDL();
        const libmdl::VVDHeader& vvdData = model.getVertices();
        const libmdl::VTXHeader& vtxData = model.getMeshData();

        MeshBuffer<VertexSolid, uint32> mesh;

        //
        // Materials
        //
        for (const mdl::Texture& mat : mdlData.textures(mdlData))
        {
            std::stringstream path;
            
            // Check each cdtextures dir
            for (const libmdl::ResString& cd : mdlData.textureDirs(mdlData))
            {
                path << "materials/" << cd.get(&mdlData) << mat.name.get(&mat) << ".vmt";

                if (Assets.FileExists(path.str()))
                {
                    mesh.materials.push_back(Assets.Load<Material>(path.str()));
                    break;
                }

                path.clear();
            }
        }

        // Important this does not get reallocated
        size_t numIndices = 0;
        for (const vtx::BodyPart& bodypart : vtxData.bodyParts(vtxData))
        {
            const vtx::Model& model = bodypart.models(bodypart)[0];
            const vtx::ModelLOD& lod = model.lods(model)[0];
            for (const vtx::Mesh& meshPart : lod.meshes(lod))
                for (const vtx::StripGroup& stripGroup : meshPart.stripGroups(meshPart))
                    numIndices += stripGroup.indices.count;
        }
        mesh.vertices.reserve(vvdData.numLODVertexes[0]);
        mesh.indices.reserve(numIndices);

        uint32 vertOffset = 0;

        // how many layers of nesting are you on? you are like a little baby. watch this
        int b = 0;
        for (const vtx::BodyPart& bodypart : vtxData.bodyParts(vtxData))
        {
            const mdl::BodyPart* mdlbodypart = mdlData.bodyParts.get(&mdlData, b++);
            int md = 0;
            for (const vtx::Model& model : bodypart.models(bodypart))
            {
                const mdl::Model* mdlmodel = mdlbodypart->modelData.get(mdlbodypart, md++);
                for (const vtx::ModelLOD& lod : model.lods(model))
                {
                    int ms = 0;
                    for (const vtx::Mesh& meshPart : lod.meshes(lod))
                    {
                        uint32 indexOffset = 0;

                        const mdl::Mesh* mdlmesh = mdlmodel->meshes.get(mdlmodel, ms++);
                        for (const vtx::StripGroup& stripGroup : meshPart.stripGroups(meshPart))
                        {
                            //
                            // Vertex Buffer
                            //
                            for (const vtx::Vertex& vt : stripGroup.vertices(stripGroup))
                            {
                                const vvd::Vertex& vert = vvdData.getVertex(vt.origMeshVertID + vertOffset);
                                mesh.vertices.push_back(VertexSolid {
                                    .position = vert.position,
                                    .normal = vert.normal,
                                    .uv = vec3(vert.texCoord, 0),
                                    .face = 0
                                });
                            }

                            //
                            // Index Buffer
                            //
                            auto indices = stripGroup.indices(stripGroup);
                            for (size_t i = 0; i < indices.size(); i++)
                            {
                                size_t j = i;
                                switch (j % 3) { // Swap winding order
                                    case 0: j += 2; break;
                                    case 2: j -= 2; break;
                                }
                                mesh.indices.push_back(indices[j] + indexOffset);
                            }

                            indexOffset += stripGroup.vertices.count;
                            vertOffset += stripGroup.vertices.count;
                        }

                        mesh.AddGroup(mdlmesh->material);
                        indexOffset = 0;
                    }

                    // Ignore other LODs
                    break;
                }

                // Ignore other submodels
                break;
            }
        }

        outMesh = mesh;
    }};
}
