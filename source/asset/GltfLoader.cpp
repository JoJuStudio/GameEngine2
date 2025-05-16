// source/asset/GltfLoader.cpp
#include "GltfLoader.hpp"
#include "core/Logging.hpp"
#include "renderer/SkinnedMesh.hpp"
#include "tiny_gltf.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>

namespace Asset {

    // Static member initialization
    std::vector<std::shared_ptr<Texture>> GltfLoader::s_textures;
    std::vector<std::shared_ptr<Material>> GltfLoader::s_materials;

    // Local tracker for skinned meshes (not exposed in header)
    static std::vector<std::shared_ptr<SkinnedMesh>> s_skinnedMeshes;

    std::vector<std::shared_ptr<Mesh>> GltfLoader::LoadFromFile(const std::string& filepath) {
        s_textures.clear();
        s_materials.clear();
        s_skinnedMeshes.clear();

        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err, warn;

        bool success = (filepath.rfind(".glb") != std::string::npos)
        ? loader.LoadBinaryFromFile(&model, &err, &warn, filepath)
        : loader.LoadASCIIFromFile(&model, &err, &warn, filepath);

        if (!warn.empty()) LOG_WARN("%s", warn.c_str());
        if (!err.empty()) LOG_ERROR("%s", err.c_str());
        if (!success) {
            LOG_ERROR("Failed to load GLTF file: %s", filepath.c_str());
            return {};
        }

        LoadBuffers(model);
        LoadTextures(model);
        LoadMaterials(model);

        std::vector<std::shared_ptr<Mesh>> meshes;
        LoadMeshes(model, meshes);
        LoadSkins(model, meshes);

        return meshes;
    }

    void GltfLoader::LoadBuffers(const tinygltf::Model& model) {
        for (const auto& bufView : model.bufferViews) {
            GLuint bufferId;
            glGenBuffers(1, &bufferId);
            glBindBuffer(bufView.target, bufferId);
            const auto& buffer = model.buffers[bufView.buffer];
            glBufferData(bufView.target, bufView.byteLength,
                         buffer.data.data() + bufView.byteOffset, GL_STATIC_DRAW);
        }
    }

    void GltfLoader::LoadTextures(const tinygltf::Model& model) {
        for (const auto& tex : model.textures) {
            const auto& img = model.images[tex.source];
            auto texture = std::make_shared<Texture>();
            texture->Create2D(img.width, img.height, img.component, img.image.data());
            s_textures.push_back(texture);
            LOG_INFO("Loaded texture: %dx%d", img.width, img.height);
        }
    }

    void GltfLoader::LoadMaterials(const tinygltf::Model& model) {
        for (const auto& mat : model.materials) {
            auto material = std::make_shared<Material>();
            auto& pbr = mat.pbrMetallicRoughness;

            if (pbr.baseColorFactor.size() == 4) {
                material->SetBaseColorFactor({
                    static_cast<float>(pbr.baseColorFactor[0]),
                                             static_cast<float>(pbr.baseColorFactor[1]),
                                             static_cast<float>(pbr.baseColorFactor[2]),
                                             static_cast<float>(pbr.baseColorFactor[3])
                });
            }

            material->SetMetallicFactor(static_cast<float>(pbr.metallicFactor));
            material->SetRoughnessFactor(static_cast<float>(pbr.roughnessFactor));

            if (pbr.baseColorTexture.index >= 0) {
                material->SetBaseColorTexture(s_textures[pbr.baseColorTexture.index]);
            }

            s_materials.push_back(material);
        }
    }

    void GltfLoader::LoadMeshes(const tinygltf::Model& model, std::vector<std::shared_ptr<Mesh>>& outMeshes) {
        for (const auto& mesh : model.meshes) {
            for (const auto& prim : mesh.primitives) {
                const bool isSkinned = prim.attributes.count("JOINTS_0") && prim.attributes.count("WEIGHTS_0");

                std::shared_ptr<Mesh> meshPtr;
                if (isSkinned) {
                    auto skinned = std::make_shared<SkinnedMesh>();
                    s_skinnedMeshes.push_back(skinned);
                    meshPtr = skinned;
                } else {
                    meshPtr = std::make_shared<Mesh>();
                }

                GLuint posVBO = 0, normVBO = 0, texVBO = 0;

                // Load POSITION attribute
                if (prim.attributes.count("POSITION")) {
                    const auto& acc = model.accessors[prim.attributes.at("POSITION")];
                    const auto& view = model.bufferViews[acc.bufferView];
                    const auto& buffer = model.buffers[view.buffer];
                    const float* ptr = reinterpret_cast<const float*>(
                        buffer.data.data() + view.byteOffset + acc.byteOffset);

                    glGenBuffers(1, &posVBO);
                    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
                    glBufferData(GL_ARRAY_BUFFER, acc.count * 3 * sizeof(float), ptr, GL_STATIC_DRAW);
                }

                // Load NORMAL attribute
                if (prim.attributes.count("NORMAL")) {
                    const auto& acc = model.accessors[prim.attributes.at("NORMAL")];
                    const auto& view = model.bufferViews[acc.bufferView];
                    const auto& buffer = model.buffers[view.buffer];
                    const float* ptr = reinterpret_cast<const float*>(
                        buffer.data.data() + view.byteOffset + acc.byteOffset);

                    glGenBuffers(1, &normVBO);
                    glBindBuffer(GL_ARRAY_BUFFER, normVBO);
                    glBufferData(GL_ARRAY_BUFFER, acc.count * 3 * sizeof(float), ptr, GL_STATIC_DRAW);
                }

                // Load TEXCOORD_0 attribute
                if (prim.attributes.count("TEXCOORD_0")) {
                    const auto& acc = model.accessors[prim.attributes.at("TEXCOORD_0")];
                    const auto& view = model.bufferViews[acc.bufferView];
                    const auto& buffer = model.buffers[view.buffer];
                    const float* ptr = reinterpret_cast<const float*>(
                        buffer.data.data() + view.byteOffset + acc.byteOffset);

                    glGenBuffers(1, &texVBO);
                    glBindBuffer(GL_ARRAY_BUFFER, texVBO);
                    glBufferData(GL_ARRAY_BUFFER, acc.count * 2 * sizeof(float), ptr, GL_STATIC_DRAW);
                }

                // Handle skinned mesh specific data
                if (isSkinned) {
                    auto skinned = std::static_pointer_cast<SkinnedMesh>(meshPtr);

                    // Load joint indices
                    std::vector<uint16_t> jointsU16;
                    const uint16_t* joints = nullptr;
                    size_t vertexCount = 0;
                    if (prim.attributes.count("JOINTS_0")) {
                        const auto& acc = model.accessors[prim.attributes.at("JOINTS_0")];
                        const auto& view = model.bufferViews[acc.bufferView];
                        const auto& buffer = model.buffers[view.buffer];

                        if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                            const uint8_t* jointsU8 = reinterpret_cast<const uint8_t*>(
                                buffer.data.data() + view.byteOffset + acc.byteOffset);
                            jointsU16.resize(acc.count * 4);
                            for (size_t i = 0; i < acc.count * 4; ++i) {
                                jointsU16[i] = static_cast<uint16_t>(jointsU8[i]);
                            }
                            joints = jointsU16.data();
                        } else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                            joints = reinterpret_cast<const uint16_t*>(
                                buffer.data.data() + view.byteOffset + acc.byteOffset);
                        }
                        vertexCount = acc.count;
                    }

                    // Load weights
                    const float* weights = nullptr;
                    if (prim.attributes.count("WEIGHTS_0")) {
                        const auto& acc = model.accessors[prim.attributes.at("WEIGHTS_0")];
                        const auto& view = model.bufferViews[acc.bufferView];
                        const auto& buffer = model.buffers[view.buffer];
                        weights = reinterpret_cast<const float*>(
                            buffer.data.data() + view.byteOffset + acc.byteOffset);
                    }

                    // Load indices
                    const uint16_t* indices = nullptr;
                    size_t indexCount = 0;
                    if (prim.indices >= 0) {
                        const auto& acc = model.accessors[prim.indices];
                        const auto& view = model.bufferViews[acc.bufferView];
                        const auto& buffer = model.buffers[view.buffer];
                        indices = reinterpret_cast<const uint16_t*>(
                            buffer.data.data() + view.byteOffset + acc.byteOffset);
                        indexCount = acc.count;
                    }

                    skinned->InitBuffers(posVBO, normVBO, texVBO, joints, weights, indices, indexCount, vertexCount);
                }

                // Assign material
                if (prim.material >= 0 && prim.material < static_cast<int>(s_materials.size())) {
                    meshPtr->SetMaterial(s_materials[prim.material]);
                }

                outMeshes.push_back(meshPtr);
            }
        }
    }

    void GltfLoader::LoadSkins(const tinygltf::Model& model, std::vector<std::shared_ptr<Mesh>>& meshes) {
        for (const auto& skin : model.skins) {
            if (skin.inverseBindMatrices < 0 || skin.joints.empty()) continue;

            // Load inverse bind matrices
            const auto& accessor = model.accessors[skin.inverseBindMatrices];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];
            const float* ibmData = reinterpret_cast<const float*>(
                buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

            std::vector<Bone> bones;
            bones.reserve(skin.joints.size());

            // Create bones
            for (size_t i = 0; i < skin.joints.size(); ++i) {
                const int nodeIndex = skin.joints[i];
                const auto& node = model.nodes[nodeIndex];

                Bone bone;
                bone.name = node.name;
                bone.parentIndex = -1;

                // Calculate local transform
                glm::mat4 translation = glm::mat4(1.0f);
                if (!node.translation.empty()) {
                    translation = glm::translate(translation,
                                                 glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
                }

                glm::mat4 rotation = glm::mat4(1.0f);
                if (!node.rotation.empty()) {
                    glm::quat q(
                        node.rotation[3],  // w
                        node.rotation[0],  // x
                        node.rotation[1],  // y
                        node.rotation[2]   // z
                    );
                    rotation = glm::toMat4(q);
                }

                glm::mat4 scale = glm::mat4(1.0f);
                if (!node.scale.empty()) {
                    scale = glm::scale(scale,
                                       glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
                }

                bone.localTransform = translation * rotation * scale;
                bone.inverseBindMatrix = glm::make_mat4(ibmData + i * 16);
                bones.push_back(bone);
            }

            // Find parent indices by checking children
            for (size_t i = 0; i < skin.joints.size(); ++i) {
                const int currentJoint = skin.joints[i];

                // Check all nodes in the skin for parent relationship
                for (size_t j = 0; j < skin.joints.size(); ++j) {
                    const auto& potentialParent = model.nodes[skin.joints[j]];
                    for (int child : potentialParent.children) {
                        if (child == currentJoint) {
                            bones[i].parentIndex = j;
                            break;
                        }
                    }
                }
            }

            // Assign bones to all tracked skinned meshes
            for (auto& skinned : s_skinnedMeshes) {
                skinned->SetBones(bones);
                LOG_INFO("Assigned %zu bones to SkinnedMesh", bones.size());
            }
        }
    }

    std::vector<std::shared_ptr<AnimationClip>> GltfLoader::LoadAnimations(const std::string& filepath) {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err, warn;

        bool success = (filepath.rfind(".glb") != std::string::npos)
        ? loader.LoadBinaryFromFile(&model, &err, &warn, filepath)
        : loader.LoadASCIIFromFile(&model, &err, &warn, filepath);

        if (!warn.empty()) LOG_WARN("%s", warn.c_str());
        if (!err.empty()) LOG_ERROR("%s", err.c_str());
        if (!success) {
            LOG_ERROR("Failed to load GLTF animations from file: %s", filepath.c_str());
            return {};
        }

        std::vector<std::shared_ptr<AnimationClip>> clips;

        for (const auto& anim : model.animations) {
            auto clip = std::make_shared<AnimationClip>();
            clip->SetName(anim.name.empty() ? "unnamed_animation" : anim.name);

            for (const auto& channel : anim.channels) {
                const auto& sampler = anim.samplers[channel.sampler];
                const std::string boneName = model.nodes[channel.target_node].name;

                // Load time values
                const auto& inputAccessor = model.accessors[sampler.input];
                const auto& inputView = model.bufferViews[inputAccessor.bufferView];
                const auto& inputBuffer = model.buffers[inputView.buffer];
                const float* times = reinterpret_cast<const float*>(
                    inputBuffer.data.data() + inputView.byteOffset + inputAccessor.byteOffset);

                // Load keyframe values
                const auto& outputAccessor = model.accessors[sampler.output];
                const auto& outputView = model.bufferViews[outputAccessor.bufferView];
                const auto& outputBuffer = model.buffers[outputView.buffer];
                const float* values = reinterpret_cast<const float*>(
                    outputBuffer.data.data() + outputView.byteOffset + outputAccessor.byteOffset);

                // Process keyframes
                for (size_t i = 0; i < inputAccessor.count; ++i) {
                    AnimationClip::Keyframe keyframe;
                    keyframe.time = times[i];

                    if (channel.target_path == "translation") {
                        keyframe.position = glm::vec3(
                            values[i * 3 + 0],
                            values[i * 3 + 1],
                            values[i * 3 + 2]
                        );
                    } else if (channel.target_path == "rotation") {
                        keyframe.rotation = glm::quat(
                            values[i * 4 + 3],  // w
                            values[i * 4 + 0],  // x
                            values[i * 4 + 1],  // y
                            values[i * 4 + 2]   // z
                        );
                    } else if (channel.target_path == "scale") {
                        keyframe.scale = glm::vec3(
                            values[i * 3 + 0],
                            values[i * 3 + 1],
                            values[i * 3 + 2]
                        );
                    }

                    clip->AddKeyframe(boneName, keyframe);
                }
            }

            clips.push_back(clip);
            LOG_INFO("Loaded animation: %s (%zu channels)", clip->GetName().c_str(), anim.channels.size());
        }

        return clips;
    }

} // namespace Asset
