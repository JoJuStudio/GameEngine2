#pragma once
#include "Component.hpp"
#include <memory>
#include <string_view>
#include <vector>

class Transform;

class GameObject {
public:
    explicit GameObject(std::string_view name = "GameObject");
    ~GameObject();

    // --- hierarchy ---
    GameObject& CreateChild(std::string_view name = "Child");
    GameObject* Parent() const { return m_parent; }
    std::span<std::unique_ptr<GameObject>> Children() { return m_children; }

    // --- components ---
    template <class T, class... Args>
    T& AddComponent(Args&&... args);
    template <class T>
    T* GetComponent() const;

    // --- ticking ---
    void Update(float dt);

    Transform& TransformComponent(); // convenience accessor

private:
    std::string m_name;
    GameObject* m_parent { nullptr };

    std::vector<std::unique_ptr<Component>> m_components;
    std::vector<std::unique_ptr<GameObject>> m_children;
};
