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

    // Now that we have bones, validate the joint indices
    if (!m_jointIndices.empty()) {
        int maxBoneIndex = static_cast<int>(m_bones.size()) - 1;
        for (size_t i = 0; i < m_vertexCount; ++i) {
            const uint16_t* j = &m_jointIndices[i * 4];
            const float* w = &m_jointWeights[i * 4];
            float total = w[0] + w[1] + w[2] + w[3];

            if (total < 0.001f) {
                LOG_WARN("Bad weights at vertex %zu: [%.2f %.2f %.2f %.2f] (sum=%.2f)", i, w[0], w[1], w[2], w[3], total);
            }

            if (j[0] > maxBoneIndex || j[1] > maxBoneIndex || j[2] > maxBoneIndex || j[3] > maxBoneIndex) {
                LOG_WARN("Out-of-range joint index at vertex %zu: [%u %u %u %u] (max allowed: %d)",
                         i, j[0], j[1], j[2], j[3], maxBoneIndex);
            }
        }

        // Clear the stored data since we don't need it anymore
        std::vector<uint16_t>().swap(m_jointIndices);
        std::vector<float>().swap(m_jointWeights);
    }

    LOG_INFO("Assigned %zu bones to SkinnedMesh", bones.size());
}

void SkinnedMesh::UpdateBoneTransforms()
{
    if (m_bones.empty()) {
        LOG_WARN("UpdateBoneTransforms called with no bones");
        return;
    }

    // First pass: update global transforms
    for (size_t i = 0; i < m_bones.size(); ++i) {
        const auto& bone = m_bones[i];

        // Validate parent index
        if (bone.parentIndex >= static_cast<int>(m_bones.size())) {
            LOG_ERROR("Bone %zu has invalid parent index %d", i, bone.parentIndex);
            continue;
        }

        if (bone.parentIndex >= 0) {
            // Validate that we're not creating a cycle
            int parentIdx = bone.parentIndex;
            int maxDepth = static_cast<int>(m_bones.size());
            int depth = 0;
            while (parentIdx >= 0 && depth < maxDepth) {
                if (parentIdx == static_cast<int>(i)) {
                    LOG_ERROR("Bone hierarchy has a cycle at bone %zu", i);
                    break;
                }
                parentIdx = m_bones[parentIdx].parentIndex;
                depth++;
            }

            m_bones[i].globalTransform = m_bones[bone.parentIndex].globalTransform * bone.localTransform;
        } else {
            m_bones[i].globalTransform = bone.localTransform;
        }
    }

    // Second pass: compute final matrices
    for (size_t i = 0; i < m_bones.size(); ++i) {
        const auto& bone = m_bones[i];

        // Validate inverse bind matrix
        if (i < m_inverseBindMatrices.size()) {
            m_finalBoneMatrices[i] = m_bones[i].globalTransform * m_inverseBindMatrices[i];
        } else {
            m_finalBoneMatrices[i] = m_bones[i].globalTransform * bone.inverseBindMatrix;
        }

        // Check for invalid transforms
        const float* mat = glm::value_ptr(m_finalBoneMatrices[i]);
        bool hasNaN = false;
        bool hasInf = false;
        for (int j = 0; j < 16; ++j) {
            if (std::isnan(mat[j])) hasNaN = true;
            if (std::isinf(mat[j])) hasInf = true;
        }

        if (hasNaN || hasInf) {
            LOG_ERROR("Bone[%zu] has invalid transform (NaN=%d, Inf=%d)", i, hasNaN, hasInf);
            m_finalBoneMatrices[i] = glm::mat4(1.0f);  // Reset to identity
        }
    }

    static bool logged = false;
    if (!logged) {
        LOG_INFO("Final bone matrices:");
        for (size_t i = 0; i < std::min<size_t>(3, m_finalBoneMatrices.size()); ++i) {
            const float* mat = glm::value_ptr(m_finalBoneMatrices[i]);
            LOG_INFO("Bone[%zu] = [%.2f %.2f %.2f %.2f | %.2f %.2f %.2f %.2f | %.2f %.2f %.2f %.2f | %.2f %.2f %.2f %.2f]",
                i,
                mat[0], mat[1], mat[2], mat[3],
                mat[4], mat[5], mat[6], mat[7],
                mat[8], mat[9], mat[10], mat[11],
                mat[12], mat[13], mat[14], mat[15]);
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
    LOG_INFO("SkinnedMesh::InitBuffers - VAO created: %u", m_vao);

    m_indexCount = indexCount;
    m_vertexCount = vertexCount;

    // Store joint indices and weights for later validation
    m_jointIndices.resize(vertexCount * 4);
    m_jointWeights.resize(vertexCount * 4);
    if (joints) {
        std::memcpy(m_jointIndices.data(), joints, vertexCount * 4 * sizeof(uint16_t));
    }
    if (weights) {
        std::memcpy(m_jointWeights.data(), weights, vertexCount * 4 * sizeof(float));
    }

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
    static bool drewOnce = false;
    if (!drewOnce) {
        LOG_INFO("SkinnedMesh::Draw() called with %zu bones", m_finalBoneMatrices.size());
        drewOnce = true;
    }

    // Validate bone matrices before uploading
    if (m_finalBoneMatrices.empty()) {
        LOG_ERROR("No bone matrices available for skinned mesh");
        return;
    }

    if (m_finalBoneMatrices.size() > 100) {  // Match shader's max bones
        LOG_ERROR("Too many bones (%zu) for shader (max 100)", m_finalBoneMatrices.size());
        return;
    }

    // Upload bone matrices
    GLint bonesLoc = glGetUniformLocation(shaderProgram, "uBones");
    if (bonesLoc != -1) {
        // Validate that all matrices are good before uploading
        bool hasInvalidMatrix = false;
        for (const auto& mat : m_finalBoneMatrices) {
            const float* data = glm::value_ptr(mat);
            for (int i = 0; i < 16; ++i) {
                if (std::isnan(data[i]) || std::isinf(data[i])) {
                    hasInvalidMatrix = true;
                    break;
                }
            }
            if (hasInvalidMatrix) break;
        }

        if (hasInvalidMatrix) {
            LOG_ERROR("Invalid bone matrices detected, skipping upload");
            return;
        }

        glUniformMatrix4fv(bonesLoc, static_cast<GLsizei>(m_finalBoneMatrices.size()),
            GL_FALSE, glm::value_ptr(m_finalBoneMatrices[0]));
    } else {
        LOG_ERROR("Uniform 'uBones' not found in shader program %u", shaderProgram);
        return;
    }

    // Bind material
    if (m_material) {
        if (m_material->GetBaseColorTexture()) {
            m_material->Bind(shaderProgram);
        } else {
            LOG_WARN("Material has no base color texture");
        }
    } else {
        LOG_WARN("SkinnedMesh has no material");
    }

    // Validate VAO before drawing
    GLint currentVAO = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
    if (currentVAO != 0) {
        LOG_WARN("Another VAO was bound (%d), unbinding it", currentVAO);
        glBindVertexArray(0);
    }

    // Bind and draw
    glBindVertexArray(m_vao);

    // Validate vertex attributes
    GLint maxAttrib = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttrib);
    for (GLint i = 0; i < 5; ++i) {  // We use attributes 0-4
        GLint enabled = 0;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        if (!enabled) {
            LOG_ERROR("Required vertex attribute %d not enabled", i);
            glBindVertexArray(0);
            return;
        }
    }

    if (IndexCount() > 0) {
        if (IndexCount() % 3 != 0) {
            LOG_WARN("Index count %zu is not a multiple of 3", IndexCount());
        }
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(IndexCount()), GL_UNSIGNED_SHORT, nullptr);
    } else if (VertexCount() > 0) {
        if (VertexCount() % 3 != 0) {
            LOG_WARN("Vertex count %zu is not a multiple of 3", VertexCount());
        }
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(VertexCount()));
    } else {
        LOG_ERROR("No vertices or indices to draw");
    }

    glBindVertexArray(0);

    // Check for GL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        LOG_ERROR("GL error in SkinnedMesh::Draw: 0x%x", err);
    }
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
