#include "Texture.hpp"
// include your GL loader; if you use GLUtils.hpp to pull in GLAD, include that:
#include "../graphics/GLUtils.hpp"
// otherwise you can do: #include <glad/glad.h>

Texture::Texture()
{
    glGenTextures(1, &m_id);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

void Texture::Create2D(int width, int height, int components, const unsigned char* data)
{
    glBindTexture(GL_TEXTURE_2D, m_id);

    GLenum format = GL_RGBA;
    if (components == 1)
        format = GL_RED;
    else if (components == 2)
        format = GL_RG;
    else if (components == 3)
        format = GL_RGB;
    // else keep GL_RGBA for 4

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}
