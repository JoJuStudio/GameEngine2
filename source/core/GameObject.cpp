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
    OnUpdate(dt); // <-- Call custom per-object update before components

    for (auto& c : m_components)
        c->update(dt);
    for (auto& ch : m_children)
        ch->update(dt);
}

void GameObject::draw()
{
    OnDraw(); // per‐object hook

    // first draw all my components
    for (auto& comp : m_components)
        comp->draw();

    // then draw my children
    for (auto& ch : m_children)
        ch->draw();
}
Transform& GameObject::transform()
{
    return *getComponent<Transform>();
}

/* explicit instantiations so linker keeps symbols */
template Transform& GameObject::addComponent<Transform, GameObject*>(GameObject*&&);
template Transform* GameObject::getComponent<Transform>() const;

// New: define default OnUpdate and OnDraw

void GameObject::OnUpdate(float /*dt*/)
{
    // Default: do nothing
}

void GameObject::OnDraw()
{
    // Default: do nothing
}
