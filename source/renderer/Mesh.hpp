#pragma once

#include <cstddef>

/// Simple wrapper around a VAO/VBO/EBO for a single mesh.
class Mesh {
public:
    Mesh();
    ~Mesh();

    /// Upload interleaved position data (x,y,z) for \p count vertices.
    void SetVertexPositions(const float* positions, std::size_t count);

    /// Upload interleaved normal data (x,y,z) for \p count vertices.
    void SetVertexNormals(const float* normals, std::size_t count);

    /// Upload index data. \p componentType is GL_UNSIGNED_SHORT or GL_UNSIGNED_INT.
    void SetIndices(const void* indices, std::size_t count, int componentType);

    std::size_t VertexCount() const { return m_vertexCount; }
    std::size_t IndexCount() const { return m_indexCount; }

    /// Bind VAO and draw elements.
    void Draw() const;

private:
    unsigned int m_vao = 0;
    unsigned int m_vboPositions = 0;
    unsigned int m_vboNormals = 0;
    unsigned int m_ebo = 0;
    std::size_t m_vertexCount = 0;
    std::size_t m_indexCount = 0;
};
