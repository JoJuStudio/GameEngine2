#include "Material.hpp"
#include "../graphics/GLUtils.hpp" // for GLUtils::checkError
#include <glad/glad.h> // for glGetUniformLocation, glUniform*

void Material::SetBaseColorFactor(const glm::vec4& c) { m_baseColorFactor = c; }
void Material::SetMetallicFactor(float m) { m_metallicFactor = m; }
void Material::SetRoughnessFactor(float r) { m_roughnessFactor = r; }

void Material::SetBaseColorTexture(std::shared_ptr<Texture> t) { m_baseColorTexture = std::move(t); }
void Material::SetMetallicRoughnessTexture(std::shared_ptr<Texture> t) { m_metallicRoughnessTexture = std::move(t); }
void Material::SetNormalTexture(std::shared_ptr<Texture> t) { m_normalTexture = std::move(t); }

void Material::Bind(GLuint program) const
{
    // Upload PBR factors
    GLint locBase = glGetUniformLocation(program, "u_BaseColorFactor");
    GLint locMetal = glGetUniformLocation(program, "u_Metallic");
    GLint locRough = glGetUniformLocation(program, "u_Roughness");

    glUniform4fv(locBase, 1, &m_baseColorFactor[0]);
    glUniform1f(locMetal, m_metallicFactor);
    glUniform1f(locRough, m_roughnessFactor);

    // Bind textures if present
    if (m_baseColorTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_baseColorTexture->GetID());
        GLint loc = glGetUniformLocation(program, "u_AlbedoMap");
        glUniform1i(loc, 0);
    }
    if (m_metallicRoughnessTexture) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_metallicRoughnessTexture->GetID());
        GLint loc = glGetUniformLocation(program, "u_MetallicRoughnessMap");
        glUniform1i(loc, 1);
    }
    if (m_normalTexture) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_normalTexture->GetID());
        GLint loc = glGetUniformLocation(program, "u_NormalMap");
        glUniform1i(loc, 2);
    }

    GLUtils::checkError("Material::Bind");
}
