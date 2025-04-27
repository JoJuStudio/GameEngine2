#pragma once

#include <memory>
#include <string>
#include <vector>

// tinygltf header (unchanged .h)
#include "tiny_gltf.h"

// Engine types
#include "../renderer/Material.hpp"
#include "../renderer/Mesh.hpp"
#include "../renderer/Texture.hpp"

// Your switch logging macros: LOG_INFO, LOG_WARN, LOG_ERROR
#include "../core/Logging.hpp"

namespace Asset {

struct GltfScene {
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Material>> materials;
};

class GltfLoader {
public:
    /// Load a .gltf or .glb file. Throws std::runtime_error on failure.
    static GltfScene LoadFromFile(const std::string& filepath);

private:
    static void LoadBuffers(const tinygltf::Model& model, GltfScene& scene);
    static void LoadTextures(const tinygltf::Model& model, GltfScene& scene);
    static void LoadMaterials(const tinygltf::Model& model, GltfScene& scene);
    static void LoadMeshes(const tinygltf::Model& model, GltfScene& scene);
};

} // namespace Asset
