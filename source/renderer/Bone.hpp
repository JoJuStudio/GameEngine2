// source/renderer/Bone.hpp

#pragma once

#include <glm/glm.hpp>

struct Bone {
    int parentIndex = -1;
    glm::mat4 localTransform = glm::mat4(1.0f);
    glm::mat4 globalTransform = glm::mat4(1.0f);
    glm::mat4 inverseBindMatrix = glm::mat4(1.0f);
};
