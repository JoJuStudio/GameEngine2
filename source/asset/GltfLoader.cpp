#include "GltfLoader.hpp"
#include "../core/Logging.hpp"
#include "tiny_gltf.h"
#include <glad/glad.h>

namespace Asset {

// Static storage
std::vector<std::shared_ptr<Texture>> GltfLoader::s_textures;
std::vector<std::shared_ptr<Material>> GltfLoader::s_materials;

std::vector<std::shared_ptr<Mesh>> GltfLoader::LoadFromFile(const std::string& filepath)
{
    s_textures.clear();
    s_materials.clear();

    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool success = (filepath.rfind(".glb") != std::string::npos)
        ? loader.LoadBinaryFromFile(&model, &err, &warn, filepath)
        : loader.LoadASCIIFromFile(&model, &err, &warn, filepath);

    if (!warn.empty())
        LOG_WARN("%s", warn.c_str());
    if (!err.empty())
        LOG_ERROR("%s", err.c_str());
    if (!success) {
        LOG_ERROR("Failed to load GLTF file: %s", filepath.c_str());
        return {};
    }

    LoadBuffers(model);
    LoadTextures(model);
    LoadMaterials(model);

    std::vector<std::shared_ptr<Mesh>> meshes;
    LoadMeshes(model, meshes);

    return meshes;
}

void GltfLoader::LoadBuffers(const tinygltf::Model& model)
{
    // Upload bufferViews to OpenGL
    for (const auto& bufView : model.bufferViews) {
        GLuint bufferId;
        glGenBuffers(1, &bufferId);
        glBindBuffer(bufView.target, bufferId);
        const auto& buffer = model.buffers[bufView.buffer];
        glBufferData(bufView.target, bufView.byteLength, buffer.data.data() + bufView.byteOffset, GL_STATIC_DRAW);
    }
}

void GltfLoader::LoadTextures(const tinygltf::Model& model)
{
    for (const auto& tex : model.textures) {
        const auto& img = model.images[tex.source];
        auto texture = std::make_shared<Texture>();
        texture->Create2D(img.width, img.height, img.component, img.image.data());
        s_textures.push_back(texture);
        LOG_INFO("Loaded texture: %dx%d", img.width, img.height);
    }
}

void GltfLoader::LoadMaterials(const tinygltf::Model& model)
{
    for (const auto& mat : model.materials) {
        auto material = std::make_shared<Material>();
        auto& pbr = mat.pbrMetallicRoughness;

        if (pbr.baseColorFactor.size() == 4) {
            material->SetBaseColorFactor({ float(pbr.baseColorFactor[0]),
                float(pbr.baseColorFactor[1]),
                float(pbr.baseColorFactor[2]),
                float(pbr.baseColorFactor[3]) });
        }
        material->SetMetallicFactor(pbr.metallicFactor);
        material->SetRoughnessFactor(pbr.roughnessFactor);

        if (pbr.baseColorTexture.index >= 0) {
            material->SetBaseColorTexture(s_textures[pbr.baseColorTexture.index]);
        }

        s_materials.push_back(material);
    }
}

void GltfLoader::LoadMeshes(const tinygltf::Model& model, std::vector<std::shared_ptr<Mesh>>& outMeshes)
{
    for (const auto& mesh : model.meshes) {
        for (const auto& prim : mesh.primitives) {
            auto meshPtr = std::make_shared<Mesh>();

            if (prim.attributes.count("POSITION")) {
                const auto& acc = model.accessors[prim.attributes.at("POSITION")];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const float* ptr = reinterpret_cast<const float*>(buffer.data.data() + view.byteOffset + acc.byteOffset);
                meshPtr->SetVertexPositions(ptr, acc.count);
            }

            if (prim.attributes.count("NORMAL")) {
                const auto& acc = model.accessors[prim.attributes.at("NORMAL")];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const float* ptr = reinterpret_cast<const float*>(buffer.data.data() + view.byteOffset + acc.byteOffset);
                meshPtr->SetVertexNormals(ptr, acc.count);
            }

            if (prim.attributes.count("TEXCOORD_0")) {
                const auto& acc = model.accessors[prim.attributes.at("TEXCOORD_0")];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const float* ptr = reinterpret_cast<const float*>(buffer.data.data() + view.byteOffset + acc.byteOffset);
                meshPtr->SetVertexTexCoords(ptr, acc.count);
            }

            if (prim.indices >= 0) {
                const auto& acc = model.accessors[prim.indices];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const void* ptr = buffer.data.data() + view.byteOffset + acc.byteOffset;
                meshPtr->SetIndices(ptr, acc.count, acc.componentType);
            }

            if (prim.material >= 0) {
                meshPtr->SetMaterial(s_materials[prim.material]);
            }

            outMeshes.push_back(meshPtr);
        }
    }
}

} // namespace Asset
