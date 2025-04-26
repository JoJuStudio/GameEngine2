#include "Transform.hpp"
#include "GameObject.hpp"
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform(GameObject* owner)
    : Component(owner)
{
}

glm::mat4 Transform::WorldMatrix() const
{
    if (!m_dirty)
        return m_cached;

    glm::mat4 M = glm::mat4(1.0f);
    M = glm::translate(M, position);
    M *= glm::mat4_cast(rotation);
    M = glm::scale(M, scale);

    if (Owner()->Parent())
        M = Owner()->Parent()->TransformComponent().WorldMatrix() * M;

    m_cached = M;
    m_dirty = false;
    return M;
}
