// source/components/GltfComponent.cpp

#include "components/GltfComponent.hpp"
#include "asset/GltfLoader.hpp"
#include "core/GameObject.hpp"
#include "core/Logging.hpp"
#include "core/Transform.hpp"
#include "graphics/Renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <memory> // For shared_ptr
#include <unordered_map> // For caching
#include <vector> // For std::vector

// Static cache for loaded GLTF meshes
static std::unordered_map<std::string, std::vector<std::shared_ptr<Mesh>>> s_loadedMeshes;

GltfComponent::GltfComponent(GameObject* owner, const std::string& path)
    : Component(owner)
{
    // Check if we already loaded this GLB before
    auto it = s_loadedMeshes.find(path);
    if (it != s_loadedMeshes.end()) {
        meshes = it->second;
        LOG_INFO("GltfComponent: Reused cached meshes from '%s'", path.c_str());
    } else {
        meshes = Asset::GltfLoader::LoadFromFile(path);
        if (meshes.empty()) {
            LOG_ERROR("GltfComponent: Failed to load meshes from '%s'", path.c_str());
        } else {
            LOG_INFO("GltfComponent: Loaded %zu mesh(es) from '%s'", meshes.size(), path.c_str());
            s_loadedMeshes[path] = meshes; // Cache the result
        }
    }
}

ComponentTypeID GltfComponent::type() const
{
    return componentTypeID<GltfComponent>();
}

void GltfComponent::draw()
{
    const auto& transform = owner()->transform();

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, transform.position);
    modelMatrix *= glm::mat4_cast(transform.rotation);
    modelMatrix = glm::scale(modelMatrix, transform.scale);

    setModelMatrix(modelMatrix); // Set to GPU for shaders

    GLint shaderProgramID = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgramID);

    for (auto& mesh : meshes) {
        auto material = mesh->GetMaterial();
        if (material && material->GetBaseColorTexture()) {
            material->Bind(shaderProgramID);
        }
        mesh->Draw(shaderProgramID);
    }
}
