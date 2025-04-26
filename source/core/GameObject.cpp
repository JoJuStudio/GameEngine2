#include "GameObject.hpp"
#include "Transform.hpp"

GameObject::GameObject(std::string_view name)
    : m_name(name)
{
    addComponent<Transform>(this);
}

GameObject::~GameObject() = default;

GameObject& GameObject::createChild(std::string_view name)
{
    auto& uptr = m_children.emplace_back(std::make_unique<GameObject>(name));
    uptr->m_parent = this;
    return *uptr;
}

void GameObject::update(float dt)
{
    for (auto& c : m_components)
        c->update(dt);
    for (auto& ch : m_children)
        ch->update(dt);
}

Transform& GameObject::transform()
{
    return *getComponent<Transform>();
}

/* explicit instantiations so linker keeps symbols */
template Transform& GameObject::addComponent<Transform, GameObject*>(GameObject*&&);
template Transform* GameObject::getComponent<Transform>() const;
