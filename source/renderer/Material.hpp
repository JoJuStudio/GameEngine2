#pragma once

#include "Texture.hpp"
#include <glad/glad.h> // for GLuint
#include <glm/glm.hpp>
#include <memory>

/// A simple PBR material that binds uniforms and textures into a GL program.
class Material {
public:
    Material() = default;

    // Set PBR factors
    void SetBaseColorFactor(const glm::vec4& color);
    void SetMetallicFactor(float metallic);
    void SetRoughnessFactor(float roughness);

    // Assign textures
    void SetBaseColorTexture(std::shared_ptr<Texture> t);
    void SetMetallicRoughnessTexture(std::shared_ptr<Texture> t);
    void SetNormalTexture(std::shared_ptr<Texture> t);

    /// Bind all material parameters into the given GL program ID
    void Bind(GLuint program) const;

    // Accessors
    const glm::vec4& GetBaseColorFactor() const { return m_baseColorFactor; }
    float GetMetallicFactor() const { return m_metallicFactor; }
    float GetRoughnessFactor() const { return m_roughnessFactor; }
    std::shared_ptr<Texture> GetBaseColorTexture() const { return m_baseColorTexture; }
    std::shared_ptr<Texture> GetMetallicRoughnessTexture() const { return m_metallicRoughnessTexture; }
    std::shared_ptr<Texture> GetNormalTexture() const { return m_normalTexture; }

private:
    glm::vec4 m_baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float m_metallicFactor = 1.0f;
    float m_roughnessFactor = 1.0f;

    std::shared_ptr<Texture> m_baseColorTexture;
    std::shared_ptr<Texture> m_metallicRoughnessTexture;
    std::shared_ptr<Texture> m_normalTexture;
};
