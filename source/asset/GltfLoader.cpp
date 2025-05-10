// source/asset/GltfLoader.cpp

#include "GltfLoader.hpp"
#include "core/Logging.hpp"
#include "renderer/SkinnedMesh.hpp"
#include "tiny_gltf.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Asset {

std::vector<std::shared_ptr<Texture>> GltfLoader::s_textures;
std::vector<std::shared_ptr<Material>> GltfLoader::s_materials;

std::vector<std::shared_ptr<Mesh>> GltfLoader::LoadFromFile(const std::string& filepath)
{
    s_textures.clear();
    s_materials.clear();

    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool success = (filepath.rfind(".glb") != std::string::npos)
        ? loader.LoadBinaryFromFile(&model, &err, &warn, filepath)
        : loader.LoadASCIIFromFile(&model, &err, &warn, filepath);

    if (!warn.empty())
        LOG_WARN("%s", warn.c_str());
    if (!err.empty())
        LOG_ERROR("%s", err.c_str());
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

void GltfLoader::LoadBuffers(const tinygltf::Model& model)
{
    for (const auto& bufView : model.bufferViews) {
        GLuint bufferId;
        glGenBuffers(1, &bufferId);
        glBindBuffer(bufView.target, bufferId);
        const auto& buffer = model.buffers[bufView.buffer];
        glBufferData(bufView.target, bufView.byteLength, buffer.data.data() + bufView.byteOffset, GL_STATIC_DRAW);
    }
}

void GltfLoader::LoadTextures(const tinygltf::Model& model)
{
    for (const auto& tex : model.textures) {
        const auto& img = model.images[tex.source];
        auto texture = std::make_shared<Texture>();
        texture->Create2D(img.width, img.height, img.component, img.image.data());
        s_textures.push_back(texture);
        LOG_INFO("Loaded texture: %dx%d", img.width, img.height);
    }
}

void GltfLoader::LoadMaterials(const tinygltf::Model& model)
{
    for (const auto& mat : model.materials) {
        auto material = std::make_shared<Material>();
        auto& pbr = mat.pbrMetallicRoughness;

        if (pbr.baseColorFactor.size() == 4) {
            material->SetBaseColorFactor({ float(pbr.baseColorFactor[0]),
                float(pbr.baseColorFactor[1]),
                float(pbr.baseColorFactor[2]),
                float(pbr.baseColorFactor[3]) });
        }
        material->SetMetallicFactor(pbr.metallicFactor);
        material->SetRoughnessFactor(pbr.roughnessFactor);

        if (pbr.baseColorTexture.index >= 0) {
            material->SetBaseColorTexture(s_textures[pbr.baseColorTexture.index]);
        }

        s_materials.push_back(material);
    }
}

void GltfLoader::LoadMeshes(const tinygltf::Model& model, std::vector<std::shared_ptr<Mesh>>& outMeshes)
{
    for (const auto& mesh : model.meshes) {
        for (const auto& prim : mesh.primitives) {
            bool isSkinned = prim.attributes.count("JOINTS_0") && prim.attributes.count("WEIGHTS_0");

            std::shared_ptr<Mesh> meshPtr;
            if (isSkinned) {
                meshPtr = std::make_shared<SkinnedMesh>();
            } else {
                meshPtr = std::make_shared<Mesh>();
            }

            if (prim.attributes.count("POSITION")) {
                const auto& acc = model.accessors[prim.attributes.at("POSITION")];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const float* ptr = reinterpret_cast<const float*>(buffer.data.data() + view.byteOffset + acc.byteOffset);
                meshPtr->SetVertexPositions(ptr, acc.count);
            }

            if (prim.attributes.count("NORMAL")) {
                const auto& acc = model.accessors[prim.attributes.at("NORMAL")];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const float* ptr = reinterpret_cast<const float*>(buffer.data.data() + view.byteOffset + acc.byteOffset);
                meshPtr->SetVertexNormals(ptr, acc.count);
            }

            if (prim.attributes.count("TEXCOORD_0")) {
                const auto& acc = model.accessors[prim.attributes.at("TEXCOORD_0")];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const float* ptr = reinterpret_cast<const float*>(buffer.data.data() + view.byteOffset + acc.byteOffset);
                meshPtr->SetVertexTexCoords(ptr, acc.count);
            }

            if (isSkinned) {
                auto skinned = std::static_pointer_cast<SkinnedMesh>(meshPtr);

                if (prim.attributes.count("JOINTS_0")) {
                    const auto& acc = model.accessors[prim.attributes.at("JOINTS_0")];
                    const auto& view = model.bufferViews[acc.bufferView];
                    const auto& buffer = model.buffers[view.buffer];
                    const uint16_t* ptr = reinterpret_cast<const uint16_t*>(buffer.data.data() + view.byteOffset + acc.byteOffset);
                    skinned->SetVertexJointIndices(ptr, acc.count);
                }

                if (prim.attributes.count("WEIGHTS_0")) {
                    const auto& acc = model.accessors[prim.attributes.at("WEIGHTS_0")];
                    const auto& view = model.bufferViews[acc.bufferView];
                    const auto& buffer = model.buffers[view.buffer];
                    const float* ptr = reinterpret_cast<const float*>(buffer.data.data() + view.byteOffset + acc.byteOffset);
                    skinned->SetVertexWeights(ptr, acc.count);
                }
            }

            if (prim.indices >= 0) {
                const auto& acc = model.accessors[prim.indices];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buffer = model.buffers[view.buffer];
                const void* ptr = buffer.data.data() + view.byteOffset + acc.byteOffset;
                meshPtr->SetIndices(ptr, acc.count, acc.componentType);
            }

            if (prim.material >= 0) {
                meshPtr->SetMaterial(s_materials[prim.material]);
            }

            outMeshes.push_back(meshPtr);
        }
    }
}

void GltfLoader::LoadSkins(const tinygltf::Model& model, std::vector<std::shared_ptr<Mesh>>& meshes)
{
    for (const auto& skin : model.skins) {
        if (skin.inverseBindMatrices < 0 || skin.joints.empty())
            continue;

        const auto& accessor = model.accessors[skin.inverseBindMatrices];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];
        const float* ibmData = reinterpret_cast<const float*>(
            buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

        std::vector<Bone> bones;

        for (size_t i = 0; i < skin.joints.size(); ++i) {
            int nodeIndex = skin.joints[i];
            const auto& node = model.nodes[nodeIndex];

            Bone bone;
            bone.name = node.name;
            bone.parentIndex = -1;

            glm::mat4 T(1.0f);
            if (!node.translation.empty()) {
                T = glm::translate(glm::mat4(1.0f), glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
            }

            glm::mat4 R(1.0f);
            if (!node.rotation.empty()) {
                glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
                R = glm::toMat4(q);
            }

            glm::mat4 S(1.0f);
            if (!node.scale.empty()) {
                S = glm::scale(glm::mat4(1.0f), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
            }

            bone.localTransform = T * R * S;
            bone.inverseBindMatrix = glm::make_mat4(ibmData + i * 16);
            bones.push_back(bone);
        }

        // Guess parent indices (naive pass)
        for (size_t i = 0; i < skin.joints.size(); ++i) {
            int boneNodeIndex = skin.joints[i];
            for (size_t j = 0; j < skin.joints.size(); ++j) {
                const auto& node = model.nodes[skin.joints[j]];
                for (int child : node.children) {
                    if (child == boneNodeIndex) {
                        bones[i].parentIndex = (int)j;
                        break;
                    }
                }
            }
        }

        for (size_t i = 0; i < meshes.size(); ++i) {
            auto skinned = std::static_pointer_cast<SkinnedMesh>(meshes[i]);
            if (skinned) {
                skinned->SetBones(bones);
                LOG_INFO("Assigned %zu bones to SkinnedMesh %zu", bones.size(), i);
            }
        }
    }
}

std::vector<std::shared_ptr<AnimationClip>> GltfLoader::LoadAnimations(const std::string& filepath)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool success = (filepath.rfind(".glb") != std::string::npos)
        ? loader.LoadBinaryFromFile(&model, &err, &warn, filepath)
        : loader.LoadASCIIFromFile(&model, &err, &warn, filepath);

    if (!warn.empty())
        LOG_WARN("%s", warn.c_str());
    if (!err.empty())
        LOG_ERROR("%s", err.c_str());
    if (!success) {
        LOG_ERROR("Failed to load GLTF animations from file: %s", filepath.c_str());
        return {};
    }

    std::vector<std::shared_ptr<AnimationClip>> clips;

    for (size_t i = 0; i < model.animations.size(); ++i) {
        const auto& anim = model.animations[i];
        auto clip = std::make_shared<AnimationClip>();
        clip->SetName(anim.name.empty() ? ("anim_" + std::to_string(i)) : anim.name);

        for (const auto& channel : anim.channels) {
            const auto& sampler = anim.samplers[channel.sampler];
            std::string boneName = model.nodes[channel.target_node].name;

            const auto& inputAccessor = model.accessors[sampler.input];
            const auto& inputView = model.bufferViews[inputAccessor.bufferView];
            const auto& inputBuffer = model.buffers[inputView.buffer];
            const float* inputTimes = reinterpret_cast<const float*>(
                inputBuffer.data.data() + inputView.byteOffset + inputAccessor.byteOffset);

            const auto& outputAccessor = model.accessors[sampler.output];
            const auto& outputView = model.bufferViews[outputAccessor.bufferView];
            const auto& outputBuffer = model.buffers[outputView.buffer];
            const float* outputValues = reinterpret_cast<const float*>(
                outputBuffer.data.data() + outputView.byteOffset + outputAccessor.byteOffset);

            for (size_t i = 0; i < inputAccessor.count; ++i) {
                AnimationClip::Keyframe key;
                key.time = inputTimes[i];

                if (channel.target_path == "translation") {
                    key.position = glm::vec3(outputValues[i * 3 + 0], outputValues[i * 3 + 1], outputValues[i * 3 + 2]);
                } else if (channel.target_path == "rotation") {
                    key.rotation = glm::quat(outputValues[i * 4 + 3], outputValues[i * 4 + 0],
                        outputValues[i * 4 + 1], outputValues[i * 4 + 2]);
                } else if (channel.target_path == "scale") {
                    key.scale = glm::vec3(outputValues[i * 3 + 0], outputValues[i * 3 + 1], outputValues[i * 3 + 2]);
                }

                clip->AddKeyframe(boneName, key); // ✅ now valid
            }
        }

        LOG_INFO("Found animation %zu: '%s'", i, clip->GetName().c_str());
        clips.push_back(clip);
    }

    return clips;
}

} // namespace Asset
