// source/components/GltfComponent.hpp
#pragma once

#include "asset/GltfLoader.hpp"
#include "core/Component.hpp"
#include "renderer/Mesh.hpp"

#include <memory>
#include <string>
#include <vector>

class GltfComponent : public Component {
public:
    GltfComponent(GameObject* owner, const std::string& path);

    ComponentTypeID type() const override;
    void draw() override;

private:
    std::vector<std::shared_ptr<Mesh>> meshes;
};
