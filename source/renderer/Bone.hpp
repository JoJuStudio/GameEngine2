// source/renderer/Bone.hpp

#pragma once

#include <glm/glm.hpp>
#include <string>

struct Bone {
    std::string name;
    int parentIndex = -1;
    glm::mat4 localTransform = glm::mat4(1.0f);
    glm::mat4 globalTransform = glm::mat4(1.0f);
    glm::mat4 inverseBindMatrix = glm::mat4(1.0f);
};
