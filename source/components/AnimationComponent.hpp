#pragma once

#include "components/AnimationClip.hpp"
#include "core/Component.hpp"
#include <memory>

class AnimationComponent : public Component {
public:
    AnimationComponent(GameObject* owner, std::shared_ptr<AnimationClip> clip);

    void update(float deltaTime) override;

    // ADD THIS CORRECTLY!
    ComponentTypeID type() const override;

private:
    std::shared_ptr<AnimationClip> m_clip;
    float m_currentTime = 0.0f;
};
