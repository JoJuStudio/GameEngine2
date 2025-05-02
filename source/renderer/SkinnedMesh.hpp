// source/renderer/SkinnedMesh.hpp

#pragma once

#include "Bone.hpp"
#include "Mesh.hpp"

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

class SkinnedMesh : public Mesh {
public:
    SkinnedMesh();
    virtual ~SkinnedMesh();

    void SetBones(const std::vector<Bone>& bones);
    void UpdateBoneTransforms();

    void AddInverseBindMatrix(const glm::mat4& mat);

    void SetVertexJointIndices(const uint16_t* joints, std::size_t count);
    void SetVertexWeights(const float* weights, std::size_t count);

    void Draw(GLuint shaderProgram) const override;

private:
    GLuint m_vboJoints = 0;
    GLuint m_vboWeights = 0;

    std::vector<Bone> m_bones;
    std::vector<glm::mat4> m_inverseBindMatrices;
    std::vector<glm::mat4> m_finalBoneMatrices;
};
