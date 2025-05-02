// source/components/AnimationComponent.cpp

#include "components/AnimationComponent.hpp"
#include "core/GameObject.hpp"
#include "core/Transform.hpp"

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
    m_clip->ApplyTo(owner()->transform(), m_currentTime);

    const auto& pos = owner()->transform().position;
    const auto& scl = owner()->transform().scale;
    printf("Pos: %.2f %.2f %.2f | Scale: %.2f %.2f %.2f\n", pos.x, pos.y, pos.z, scl.x, scl.y, scl.z);
}

ComponentTypeID AnimationComponent::type() const
{
    return componentTypeID<AnimationComponent>();
}
