#include "Scene.hpp"
#include "GameObject.hpp"

Scene::Scene()
    : m_root(std::make_unique<GameObject>("Root"))
{
}
Scene::~Scene() = default;

GameObject& Scene::root() { return *m_root; }

void Scene::Update(float dt) { m_root->update(dt); }
