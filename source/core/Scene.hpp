// source/core/Scene.hpp
#pragma once
#include <memory>

class GameObject;

class Scene {
public:
    Scene();
    ~Scene();

    GameObject& root();
    void Update(float dt);
    void Render();

private:
    std::unique_ptr<GameObject> m_root;
};
