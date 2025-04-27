#pragma once

#include <cstdint>

/// Simple wrapper around an OpenGL 2D texture.
class Texture {
public:
    Texture();
    ~Texture();

    /// Allocate and upload a 2D image.
    /// \param width      pixel width
    /// \param height     pixel height
    /// \param components number of channels in data (1,2,3,4)
    /// \param data       pointer to unsigned-byte pixel data
    void Create2D(int width, int height, int components, const unsigned char* data);

    /// Bind this texture to texture unit \p unit (0..).
    void Bind(unsigned int unit = 0) const;

    /// Get the raw GL texture handle.
    unsigned int GetID() const { return m_id; }

private:
    unsigned int m_id = 0;
};
