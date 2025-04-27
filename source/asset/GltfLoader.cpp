#include "GltfLoader.hpp"

#include "../core/Logging.hpp" // LOG_INFO, LOG_WARN, LOG_ERROR
#include <glad/glad.h> // for GL enums & GLuint

namespace Asset {

GltfScene GltfLoader::LoadFromFile(const std::string& filepath)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool success = (filepath.rfind(".glb") != std::string::npos)
        ? loader.LoadBinaryFromFile(&model, &err, &warn, filepath)
        : loader.LoadASCIIFromFile(&model, &err, &warn, filepath);

    if (!warn.empty()) {
        LOG_WARN("GLTF Warning from '%s': %s", filepath.c_str(), warn.c_str());
    }
    if (!err.empty()) {
        LOG_ERROR("GLTF Error in '%s': %s", filepath.c_str(), err.c_str());
    }
    if (!success) {
        LOG_ERROR("GltfLoader: failed to load '%s'", filepath.c_str());
        // return an empty scene instead of throwing
        return GltfScene {};
    }

    GltfScene scene;
    LoadBuffers(model, scene);
    LoadTextures(model, scene);
    LoadMaterials(model, scene);
    LoadMeshes(model, scene);

    LOG_INFO("Loaded glTF '%s' — meshes=%zu  materials=%zu",
        filepath.c_str(),
        scene.meshes.size(),
        scene.materials.size());

    return scene;
}

void GltfLoader::LoadBuffers(const tinygltf::Model& model, GltfScene& scene)
{
    for (const auto& bufView : model.bufferViews) {
        GLuint bufferId = 0;
        glGenBuffers(1, &bufferId);
        glBindBuffer(bufView.target, bufferId);
        const auto& buffer = model.buffers[bufView.buffer];
        const void* dataPtr = buffer.data.data() + bufView.byteOffset;
        glBufferData(bufView.target,
            bufView.byteLength,
            dataPtr,
            GL_STATIC_DRAW);
        LOG_INFO("Uploaded bufferView %d (%zu bytes) to GL buffer %u",
            bufView.buffer, bufView.byteLength, bufferId);
    }
}

void GltfLoader::LoadTextures(const tinygltf::Model& model, GltfScene& scene)
{
    for (const auto& tex : model.textures) {
        const auto& img = model.images[tex.source];
        auto texture = std::make_shared<Texture>();
        texture->Create2D(img.width,
            img.height,
            img.component,
            img.image.data());
        scene.materials.push_back(nullptr); // if you need placeholder alignment
        LOG_INFO("Loaded texture '%s': %ux%u, comp=%d",
            img.uri.c_str(), img.width, img.height, img.component);
    }
}

void GltfLoader::LoadMaterials(const tinygltf::Model& model, GltfScene& scene)
{
    scene.materials.reserve(model.materials.size());
    for (const auto& mat : model.materials) {
        auto m = std::make_shared<Material>();
        auto& pbr = mat.pbrMetallicRoughness;

        if (pbr.baseColorFactor.size() == 4) {
            m->SetBaseColorFactor({ float(pbr.baseColorFactor[0]),
                float(pbr.baseColorFactor[1]),
                float(pbr.baseColorFactor[2]),
                float(pbr.baseColorFactor[3]) });
        }
        m->SetMetallicFactor(pbr.metallicFactor);
        m->SetRoughnessFactor(pbr.roughnessFactor);

        scene.materials.push_back(m);
        LOG_INFO("Created Material[%zu]: baseColor=(%.2f,%.2f,%.2f,%.2f)  metallic=%.2f  roughness=%.2f",
            scene.materials.size() - 1,
            m->GetBaseColorFactor().r,
            m->GetBaseColorFactor().g,
            m->GetBaseColorFactor().b,
            m->GetBaseColorFactor().a,
            m->GetMetallicFactor(),
            m->GetRoughnessFactor());
    }
}

void GltfLoader::LoadMeshes(const tinygltf::Model& model, GltfScene& scene)
{
    for (const auto& mesh : model.meshes) {
        for (const auto& prim : mesh.primitives) {
            auto meshPtr = std::make_shared<Mesh>();

            // POSITION
            if (prim.attributes.count("POSITION")) {
                const auto& acc = model.accessors[prim.attributes.at("POSITION")];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const float* ptr = reinterpret_cast<const float*>(
                    buffer.data.data() + view.byteOffset + acc.byteOffset);
                meshPtr->SetVertexPositions(ptr, acc.count);
            }

            // NORMAL
            if (prim.attributes.count("NORMAL")) {
                const auto& acc = model.accessors[prim.attributes.at("NORMAL")];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const float* ptr = reinterpret_cast<const float*>(
                    buffer.data.data() + view.byteOffset + acc.byteOffset);
                meshPtr->SetVertexNormals(ptr, acc.count);
            }

            // INDICES
            if (prim.indices >= 0) {
                const auto& acc = model.accessors[prim.indices];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const void* ptr = buffer.data.data() + view.byteOffset + acc.byteOffset;
                meshPtr->SetIndices(ptr, acc.count, acc.componentType);
            }

            scene.meshes.push_back(meshPtr);
            LOG_INFO("Loaded primitive '%s': %zu verts, %zu indices",
                mesh.name.c_str(),
                meshPtr->VertexCount(),
                meshPtr->IndexCount());
        }
    }
}

} // namespace Asset
