// source/renderer/Material.cpp
#include "Material.hpp"
#include "../graphics/GLUtils.hpp" // for GLUtils::checkError
#include <glad/glad.h> // for glGetUniformLocation, glUniform*
#include <glm/gtc/type_ptr.hpp>

void Material::SetBaseColorFactor(const glm::vec4& c) { m_baseColorFactor = c; }
void Material::SetMetallicFactor(float m) { m_metallicFactor = m; }
void Material::SetRoughnessFactor(float r) { m_roughnessFactor = r; }

void Material::SetBaseColorTexture(std::shared_ptr<Texture> t) { m_baseColorTexture = std::move(t); }
void Material::SetMetallicRoughnessTexture(std::shared_ptr<Texture> t) { m_metallicRoughnessTexture = std::move(t); }
void Material::SetNormalTexture(std::shared_ptr<Texture> t) { m_normalTexture = std::move(t); }

void Material::Bind(GLuint program) const
{
    // Upload base color, metallic, roughness factors
    GLint locBase = glGetUniformLocation(program, "u_BaseColorFactor");
    if (locBase != -1)
        glUniform4fv(locBase, 1, glm::value_ptr(m_baseColorFactor));

    GLint locMetal = glGetUniformLocation(program, "u_Metallic");
    if (locMetal != -1)
        glUniform1f(locMetal, m_metallicFactor);

    GLint locRough = glGetUniformLocation(program, "u_Roughness");
    if (locRough != -1)
        glUniform1f(locRough, m_roughnessFactor);

    // Bind base color texture
    if (m_baseColorTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_baseColorTexture->GetID());
        GLint locAlbedo = glGetUniformLocation(program, "u_AlbedoMap");
        if (locAlbedo != -1)
            glUniform1i(locAlbedo, 0);
    }

    // Bind metallic-roughness texture
    if (m_metallicRoughnessTexture) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_metallicRoughnessTexture->GetID());
        GLint locMR = glGetUniformLocation(program, "u_MetallicRoughnessMap");
        if (locMR != -1)
            glUniform1i(locMR, 1);
    }

    // Bind normal map texture
    if (m_normalTexture) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_normalTexture->GetID());
        GLint locNormal = glGetUniformLocation(program, "u_NormalMap");
        if (locNormal != -1)
            glUniform1i(locNormal, 2);
    }

    GLUtils::checkError("Material::Bind");
}
