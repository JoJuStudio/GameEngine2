#include "renderer/SkinnedMesh.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>

SkinnedMesh::SkinnedMesh()
{
    glGenBuffers(1, &m_vboJoints);
    glGenBuffers(1, &m_vboWeights);
}

SkinnedMesh::~SkinnedMesh()
{
    glDeleteBuffers(1, &m_vboJoints);
    glDeleteBuffers(1, &m_vboWeights);
}

void SkinnedMesh::SetBones(const std::vector<Bone>& bones)
{
    m_bones = bones;
    m_finalBoneMatrices.resize(bones.size(), glm::mat4(1.0f));
}

void SkinnedMesh::UpdateBoneTransforms()
{
    for (size_t i = 0; i < m_bones.size(); ++i) {
        const auto& bone = m_bones[i];

        if (bone.parentIndex >= 0) {
            m_bones[i].globalTransform = m_bones[bone.parentIndex].globalTransform * bone.localTransform;
        } else {
            m_bones[i].globalTransform = bone.localTransform;
        }

        m_finalBoneMatrices[i] = m_bones[i].globalTransform * bone.inverseBindMatrix;
    }
}

void SkinnedMesh::AddInverseBindMatrix(const glm::mat4& mat)
{
    m_inverseBindMatrices.push_back(mat);
}

void SkinnedMesh::SetVertexJointIndices(const uint16_t* joints, std::size_t count)
{
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboJoints);
    glBufferData(GL_ARRAY_BUFFER, count * 4 * sizeof(uint16_t), joints, GL_STATIC_DRAW);
    glEnableVertexAttribArray(3); // layout(location = 3)
    glVertexAttribIPointer(3, 4, GL_UNSIGNED_SHORT, 0, nullptr); // integer attribute!
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SkinnedMesh::SetVertexWeights(const float* weights, std::size_t count)
{
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboWeights);
    glBufferData(GL_ARRAY_BUFFER, count * 4 * sizeof(float), weights, GL_STATIC_DRAW);
    glEnableVertexAttribArray(4); // layout(location = 4)
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SkinnedMesh::Draw(GLuint shaderProgram) const
{
    // Upload bone matrices
    for (size_t i = 0; i < m_finalBoneMatrices.size(); ++i) {
        std::string name = "uBoneMatrices[" + std::to_string(i) + "]";
        GLint loc = glGetUniformLocation(shaderProgram, name.c_str());
        if (loc >= 0) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_finalBoneMatrices[i]));
        }
    }

    Mesh::Draw(shaderProgram);
}
