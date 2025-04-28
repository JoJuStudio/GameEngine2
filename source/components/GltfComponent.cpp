// source/components/GltfComponent.cpp

#include "components/GltfComponent.hpp"
#include "core/GameObject.hpp"
#include "core/Logging.hpp"
#include "core/Transform.hpp"
#include "graphics/Renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "components/GltfComponent.hpp"
#include "core/Logging.hpp"

GltfComponent::GltfComponent(GameObject* owner, const std::string& path)
    : Component(owner)
{
    meshes = Asset::GltfLoader::LoadFromFile(path);

    if (meshes.empty()) {
        LOG_ERROR("GltfComponent: failed to load meshes from '%s'", path.c_str());
    } else {
        LOG_INFO("GltfComponent: loaded %zu mesh(es) from '%s'", meshes.size(), path.c_str());
    }
}

ComponentTypeID GltfComponent::type() const
{
    return componentTypeID<GltfComponent>();
}

void GltfComponent::draw()
{
    const auto& transform = owner()->transform();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, transform.position);
    model *= glm::mat4_cast(transform.rotation);
    model = glm::scale(model, transform.scale);

    setModelMatrix(model); // <-- NOT Renderer::SetModelMatrix!

    GLint shaderProgramID = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgramID);

    for (auto& mesh : meshes) {
        auto material = mesh->GetMaterial();
        if (material && material->GetBaseColorTexture()) {
            material->Bind(shaderProgramID); // correct Material bind
        }
        mesh->Draw(shaderProgramID);
    }
}
