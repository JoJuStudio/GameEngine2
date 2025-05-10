// source/renderer/SkinnedMesh.cpp
#include "renderer/SkinnedMesh.hpp"
#include "core/Logging.hpp"

#include <algorithm>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>

SkinnedMesh::SkinnedMesh()
{
    glGenBuffers(1, &m_vboJoints);
    glGenBuffers(1, &m_vboWeights);
    glGenBuffers(1, &m_ebo);
}

SkinnedMesh::~SkinnedMesh()
{
    glDeleteBuffers(1, &m_vboJoints);
    glDeleteBuffers(1, &m_vboWeights);
    glDeleteBuffers(1, &m_ebo);
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

    static bool logged = false;
    if (!logged) {
        for (size_t i = 0; i < std::min<size_t>(3, m_finalBoneMatrices.size()); ++i) {
            const float* mat = glm::value_ptr(m_finalBoneMatrices[i]);
            LOG_INFO("Bone[%zu] = [%.2f %.2f %.2f %.2f ...]", i, mat[0], mat[1], mat[2], mat[3]);
        }
        logged = true;
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
    glVertexAttribIPointer(3, 4, GL_UNSIGNED_SHORT, 0, nullptr);
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

void SkinnedMesh::InitBuffers(
    GLuint positionVBO, GLuint normalVBO, GLuint texCoordVBO,
    const uint16_t* joints, const float* weights,
    const uint16_t* indices, std::size_t indexCount,
    std::size_t vertexCount)
{
    m_indexCount = indexCount;

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Position -> location 0
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Normal -> location 1
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    // TexCoord -> location 2
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);

    // Joints -> location 3
    glBindBuffer(GL_ARRAY_BUFFER, m_vboJoints);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 4 * sizeof(uint16_t), joints, GL_STATIC_DRAW);
    glVertexAttribIPointer(3, 4, GL_UNSIGNED_SHORT, 0, nullptr);
    glEnableVertexAttribArray(3);

    // Weights -> location 4
    glBindBuffer(GL_ARRAY_BUFFER, m_vboWeights);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 4 * sizeof(float), weights, GL_STATIC_DRAW);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(4);

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint16_t), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void SkinnedMesh::Draw(GLuint shaderProgram) const
{
    LOG_INFO("SkinnedMesh::Draw - Using shader program: %u", shaderProgram);

    // Upload bone matrices
    GLint bonesLoc = glGetUniformLocation(shaderProgram, "uBones");
    if (bonesLoc != -1) {
        glUniformMatrix4fv(bonesLoc, static_cast<GLsizei>(m_finalBoneMatrices.size()),
            GL_FALSE, glm::value_ptr(m_finalBoneMatrices[0]));
        LOG_INFO("Uploaded %zu bone matrices to uBones", m_finalBoneMatrices.size());
    } else {
        LOG_WARN("Uniform 'uBones' not found in shader");
    }

    // Bind material
    if (m_material) {
        if (m_material->GetBaseColorTexture()) {
            LOG_INFO("Binding material base color texture");
            m_material->Bind(shaderProgram);
        } else {
            LOG_WARN("Material has no base color texture");
        }
    } else {
        LOG_WARN("SkinnedMesh has no material");
    }

    // Bind and draw
    glBindVertexArray(m_vao);
    LOG_INFO("Bound VAO: %u", m_vao);

    if (IndexCount() > 0) {
        LOG_INFO("Drawing elements: count=%zu", IndexCount());
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(IndexCount()), GL_UNSIGNED_SHORT, nullptr);
    } else if (VertexCount() > 0) {
        LOG_INFO("Drawing arrays: count=%zu", VertexCount());
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(VertexCount()));
    } else {
        LOG_WARN("No index or vertex count to draw");
    }

    glBindVertexArray(0);
}

void SkinnedMesh::ApplyAnimation(const AnimationClip& clip, float time)
{
    const auto& tracks = clip.GetTracks();

    for (size_t i = 0; i < m_bones.size(); ++i) {
        Bone& bone = m_bones[i];

        auto it = tracks.find(bone.name);
        if (it == tracks.end())
            continue;

        const auto& keyframes = it->second;

        const AnimationClip::Keyframe* prev = nullptr;
        const AnimationClip::Keyframe* next = nullptr;

        for (const auto& kf : keyframes) {
            if (kf.time >= time) {
                next = &kf;
                break;
            }
            prev = &kf;
        }

        if (!prev)
            prev = next;
        if (!next)
            next = prev;
        if (!prev || !next)
            continue;

        float t = (next == prev) ? 0.0f : (time - prev->time) / (next->time - prev->time);

        glm::vec3 position = glm::mix(prev->position, next->position, t);
        glm::quat rotation = glm::slerp(prev->rotation, next->rotation, t);
        glm::vec3 scale = glm::mix(prev->scale, next->scale, t);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::toMat4(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        bone.localTransform = T * R * S;
    }

    UpdateBoneTransforms();
}
