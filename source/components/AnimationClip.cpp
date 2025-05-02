#include "AnimationClip.hpp"
#include "../core/Transform.hpp"

void AnimationClip::ApplyTo(Transform& transform, float currentTime) const
{
    if (m_keyframes.empty())
        return;

    const Keyframe* prev = nullptr;
    const Keyframe* next = nullptr;

    for (const auto& key : m_keyframes) {
        if (key.time >= currentTime) {
            next = &key;
            break;
        }
        prev = &key;
    }

    if (!prev)
        prev = next;
    if (!next)
        next = prev;

    if (!prev || !next)
        return;

    if (prev == next) {
        transform.position = prev->position;
        transform.rotation = prev->rotation;
        transform.scale = prev->scale;
        return;
    }

    float t = (currentTime - prev->time) / (next->time - prev->time);

    transform.position = glm::mix(prev->position, next->position, t);
    transform.rotation = glm::slerp(prev->rotation, next->rotation, t);
    transform.scale = glm::mix(prev->scale, next->scale, t);
}
