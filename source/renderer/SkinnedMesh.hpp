// source/renderer/SkinnedMesh.hpp

#pragma once

#include "Bone.hpp"
#include "Mesh.hpp"
#include "components/AnimationClip.hpp"

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
    void ApplyAnimation(const AnimationClip& clip, float time);

    void SetVertexJointIndices(const uint16_t* joints, std::size_t count);
    void SetVertexWeights(const float* weights, std::size_t count);

    void InitBuffers(
        GLuint positionVBO, GLuint normalVBO, GLuint texCoordVBO,
        const uint16_t* joints, const float* weights,
        const uint16_t* indices, std::size_t indexCount,
        std::size_t vertexCount);

    void Draw(GLuint shaderProgram) const override;
    MeshType GetType() const override { return MeshType::Skinned; }

    std::size_t IndexCount() const { return m_indexCount; }

private:
    GLuint m_vboJoints = 0;
    GLuint m_vboWeights = 0;
    GLuint m_ebo = 0;

    std::size_t m_indexCount = 0;

    std::vector<Bone> m_bones;
    std::vector<glm::mat4> m_inverseBindMatrices;
    std::vector<glm::mat4> m_finalBoneMatrices;
};
