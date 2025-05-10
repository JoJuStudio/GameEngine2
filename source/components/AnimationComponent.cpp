// source/components/AnimationComponent.cpp

#include "components/AnimationComponent.hpp"
#include "components/AnimationClip.hpp"
#include "components/GltfComponent.hpp"
#include "core/GameObject.hpp"
#include "renderer/Mesh.hpp"
#include "renderer/SkinnedMesh.hpp"

AnimationComponent::AnimationComponent(GameObject* owner, std::shared_ptr<AnimationClip> clip)
    : Component(owner)
    , m_clip(clip)
{
}

void AnimationComponent::update(float deltaTime)
{
    if (!m_clip)
        return;

    m_currentTime += deltaTime;

    auto* gltf = owner()->getComponent<GltfComponent>();
    if (!gltf)
        return;

    const auto& meshes = gltf->getMeshes();
    for (const auto& mesh : meshes) {
        if (mesh->GetType() == MeshType::Skinned) {
            auto* skinned = static_cast<SkinnedMesh*>(mesh.get());
            skinned->ApplyAnimation(*m_clip, m_currentTime);
            skinned->UpdateBoneTransforms();
        }
    }
}

ComponentTypeID AnimationComponent::type() const
{
    return componentTypeID<AnimationComponent>();
}
