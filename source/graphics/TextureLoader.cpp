// source/graphics/TextureLoader.cpp

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "graphics/TextureLoader.hpp"
#include <cstdio>
#include <switch.h> // for romfs paths

GLuint loadTextureFromFile(const std::string& path)
{
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data) {
        printf("Failed to load texture: %s\n", path.c_str());
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
        0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}
