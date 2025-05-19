// source/renderer/SkinnedMesh.cpp

#include "renderer/SkinnedMesh.hpp"
#include "core/Logging.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

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

    if (!m_jointIndices.empty()) {
        // Check for mismatched data sizes
        if (m_jointIndices.size() != 4 * m_vertexCount) {
            LOG_ERROR("Joint indices size mismatch: expected %zu, got %zu", 4 * m_vertexCount, m_jointIndices.size());
            return;
        }
        if (m_jointWeights.size() != 4 * m_vertexCount) {
            LOG_ERROR("Joint weights size mismatch: expected %zu, got %zu", 4 * m_vertexCount, m_jointWeights.size());
            return;
        }

        if (bones.empty()) {
            LOG_WARN("No bones assigned, but mesh has joint indices. All vertices have invalid joint references.");
            // Optionally log per-vertex errors here if needed
        } else {
            const uint16_t maxBoneIndex = static_cast<uint16_t>(bones.size() - 1);
            for (size_t i = 0; i < m_vertexCount; ++i) {
                const uint16_t* j = &m_jointIndices[i * 4];
                const float* w = &m_jointWeights[i * 4];

                // Check weight sum
                float total = w[0] + w[1] + w[2] + w[3];
                if (total < 0.001f) {
                    LOG_WARN("Bad weights at vertex %zu: [%.2f %.2f %.2f %.2f] (sum=%.2f)", i, w[0], w[1], w[2], w[3], total);
                }

                // Check joint indices
                bool invalidIndices = false;
                for (int k = 0; k < 4; ++k) {
                    if (j[k] > maxBoneIndex) {
                        invalidIndices = true;
                        break;
                    }
                }
                if (invalidIndices) {
                    LOG_WARN("Out-of-range joint index at vertex %zu: [%u %u %u %u] (max allowed: %u)",
                             i, j[0], j[1], j[2], j[3], maxBoneIndex);
                }
            }
        }

        // Clear data if no longer needed (optional)
        m_jointIndices.clear();
        m_jointWeights.clear();
    }

    LOG_INFO("Assigned %zu bones to SkinnedMesh", bones.size());
}


void SkinnedMesh::UpdateBoneTransforms()
{
    if (m_bones.empty()) {
        LOG_WARN("UpdateBoneTransforms called with no bones");
        return;
    }

    // -- Create topological processing order --
    std::vector<size_t> processingOrder;
    {
        std::queue<size_t> boneQueue;
        std::unordered_map<int, std::vector<size_t>> parentToChildren;

        // Build parent-child relationships
        for (size_t i = 0; i < m_bones.size(); ++i) {
            int parentIdx = m_bones[i].parentIndex;
            parentToChildren[parentIdx].push_back(i);
        }

        // Start with root bones (parentIndex == -1)
        for (size_t i = 0; i < m_bones.size(); ++i) {
            if (m_bones[i].parentIndex == -1) {
                boneQueue.push(i);
            }
        }

        // Process hierarchy breadth-first
        while (!boneQueue.empty()) {
            size_t currentIdx = boneQueue.front();
            boneQueue.pop();
            processingOrder.push_back(currentIdx);

            // Add children to queue
            auto it = parentToChildren.find(static_cast<int>(currentIdx));
            if (it != parentToChildren.end()) {
                for (size_t childIdx : it->second) {
                    boneQueue.push(childIdx);
                }
            }
        }

        // Handle orphaned bones
        if (processingOrder.size() != m_bones.size()) {
            LOG_ERROR("Bone hierarchy contains %zu unprocessed bones",
                     m_bones.size() - processingOrder.size());

            std::unordered_set<size_t> processed(processingOrder.begin(), processingOrder.end());
            for (size_t i = 0; i < m_bones.size(); ++i) {
                if (!processed.count(i)) {
                    processingOrder.push_back(i);
                    LOG_ERROR("Treating bone %zu as root due to invalid parent", i);
                }
            }
        }
    }

    // -- Update global transforms --
    for (size_t boneIdx : processingOrder) {
        Bone& bone = m_bones[boneIdx];
        const int parentIdx = bone.parentIndex;

        // Validate parent index
        if (parentIdx < -1 || parentIdx >= static_cast<int>(m_bones.size())) {
            LOG_ERROR("Bone %zu has invalid parent %d (max %zu)",
                     boneIdx, parentIdx, m_bones.size());
            bone.globalTransform = bone.localTransform;
            continue;
        }

        if (parentIdx == -1) {
            // Root bone
            bone.globalTransform = bone.localTransform;
        } else {
            // Check for cycles
            int current = parentIdx;
            bool cycle = false;
            for (int i = 0; i < static_cast<int>(m_bones.size()); ++i) {
                if (current == static_cast<int>(boneIdx)) {
                    cycle = true;
                    break;
                }
                if (m_bones[current].parentIndex == -1) break;
                current = m_bones[current].parentIndex;
            }

            if (cycle) {
                LOG_ERROR("Cycle detected at bone %zu (parent %d)", boneIdx, parentIdx);
                bone.globalTransform = bone.localTransform;
            } else {
                bone.globalTransform = m_bones[parentIdx].globalTransform * bone.localTransform;
            }
        }
    }

    // -- Calculate final matrices --
    m_finalBoneMatrices.resize(m_bones.size());

    for (size_t i = 0; i < m_bones.size(); ++i) {
        const Bone& bone = m_bones[i];
        const glm::mat4& inverseBind = (i < m_inverseBindMatrices.size())
                                     ? m_inverseBindMatrices[i]
                                     : bone.inverseBindMatrix;

        m_finalBoneMatrices[i] = bone.globalTransform * inverseBind;

        // Validate matrix entries
        bool isValid = true;
        const float* data = glm::value_ptr(m_finalBoneMatrices[i]);
        for (int j = 0; j < 16; ++j) {
            if (!std::isfinite(data[j])) {
                isValid = false;
                break;
            }
        }

        if (!isValid) {
            LOG_ERROR("Invalid matrix for bone %zu (name: %s)", i, bone.name.c_str());
            m_finalBoneMatrices[i] = glm::mat4(1.0f);
        }
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

    if (m_finalBoneMatrices.empty()) {
        LOG_ERROR("No bone matrices available for skinned mesh");
        return;
    }

    constexpr GLuint MAX_BONES = 100;
    if (m_finalBoneMatrices.size() > MAX_BONES) {
        LOG_ERROR("Too many bones (%zu) for shader (max %u)", m_finalBoneMatrices.size(), MAX_BONES);
        return;
    }

    GLint bonesLoc = glGetUniformLocation(shaderProgram, "uBones");
    if (bonesLoc == -1) {
        LOG_ERROR("Uniform 'uBones' not found in shader program %u", shaderProgram);
        return;
    }

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

    // Pad bone matrices to MAX_BONES with identity
    std::vector<glm::mat4> paddedMatrices(MAX_BONES, glm::mat4(1.0f));
    std::copy(m_finalBoneMatrices.begin(), m_finalBoneMatrices.end(), paddedMatrices.begin());
    glUniformMatrix4fv(bonesLoc, MAX_BONES, GL_FALSE, glm::value_ptr(paddedMatrices[0]));

    if (m_material) {
        m_material->Bind(shaderProgram);
    } else {
        LOG_WARN("SkinnedMesh has no material");
    }

    GLint prevVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);
    glBindVertexArray(m_vao);

    // Validate required vertex attributes (example for attributes 0-4)
    GLint maxAttrib;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttrib);
    for (GLuint i = 0; i < 5; ++i) { // Adjust based on actual used attributes
        if (i >= static_cast<GLuint>(maxAttrib)) break;
        GLint enabled;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        if (!enabled) {
            LOG_ERROR("Required vertex attribute %u not enabled", i);
            glBindVertexArray(prevVAO);
            return;
        }
    }

    if (IndexCount() > 0) {
        if (IndexCount() % 3 != 0) {
            LOG_WARN("Index count %zu is not a multiple of 3", IndexCount());
        }
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(IndexCount()), m_indexType, nullptr);
    } else if (VertexCount() > 0) {
        if (VertexCount() % 3 != 0) {
            LOG_WARN("Vertex count %zu is not a multiple of 3", VertexCount());
        }
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(VertexCount()));
    } else {
        LOG_ERROR("No vertices or indices to draw");
    }

    glBindVertexArray(prevVAO);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        LOG_ERROR("GL error in SkinnedMesh::Draw: 0x%x", err);
    }
}

void SkinnedMesh::ApplyAnimation(const AnimationClip& clip, float time) {
    const auto& tracks = clip.GetTracks();

    for (size_t i = 0; i < m_bones.size(); ++i) {
        Bone& bone = m_bones[i];
        auto it = tracks.find(bone.name);
        if (it == tracks.end()) continue;

        const auto& keyframes = it->second;
        if (keyframes.empty()) continue;

        // Binary search for the first keyframe >= current time
        auto next_it = std::lower_bound(
            keyframes.begin(), keyframes.end(), time,
            [](const AnimationClip::Keyframe& kf, float t) {
                return kf.time < t;
            });

        const AnimationClip::Keyframe* prev = nullptr;
        const AnimationClip::Keyframe* next = nullptr;

        // Determine surrounding keyframes
        if (next_it != keyframes.end()) {
            next = &*next_it;
            if (next_it != keyframes.begin()) {
                prev = &*(next_it - 1);
            }
        } else {
            // Time is after last keyframe - use final frame
            prev = &keyframes.back();
        }

        // Handle edge cases
        if (!prev) prev = next;       // Time before first keyframe
        if (!next) next = prev;       // Time after last keyframe
        if (!prev || !next) continue; // Should never hit with empty check

        // Calculate interpolation factor
        float t = (prev == next) ? 0.0f
            : (time - prev->time) / (next->time - prev->time);

        // Interpolate transforms
        glm::vec3 position = glm::mix(prev->position, next->position, t);
        glm::quat rotation = glm::slerp(prev->rotation, next->rotation, t);
        glm::vec3 scale = glm::mix(prev->scale, next->scale, t);

        // Compose transformation matrix
        bone.localTransform =
            glm::translate(glm::mat4(1.0f), position) *
            glm::toMat4(rotation) *
            glm::scale(glm::mat4(1.0f), scale);
    }

    UpdateBoneTransforms();
}
