// source/asset/GltfLoader.hpp

#pragma once

#include "components/AnimationClip.hpp"
#include "renderer/Material.hpp"
#include "renderer/Mesh.hpp"
#include "renderer/Texture.hpp"
#include "tiny_gltf.h"

#include <memory>
#include <string>
#include <vector>

namespace Asset {

class GltfLoader {
public:
    static std::vector<std::shared_ptr<Mesh>> LoadFromFile(const std::string& filepath);

    static std::vector<std::shared_ptr<AnimationClip>> LoadAnimations(const std::string& filepath);

private:
    static void LoadBuffers(const tinygltf::Model& model);
    static void LoadTextures(const tinygltf::Model& model);
    static void LoadMaterials(const tinygltf::Model& model);
    static void LoadMeshes(const tinygltf::Model& model, std::vector<std::shared_ptr<Mesh>>& outMeshes);

    // ✅ ADD THIS:
    static void LoadSkins(const tinygltf::Model& model, std::vector<std::shared_ptr<Mesh>>& outMeshes);

    static std::vector<std::shared_ptr<Texture>> s_textures;
    static std::vector<std::shared_ptr<Material>> s_materials;
};

} // namespace Asset
