#include "Mesh.hpp"
#include "../graphics/GLUtils.hpp" // pulls in <glad/glad.h> or equivalent

Mesh::Mesh()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vboPositions);
    glGenBuffers(1, &m_vboNormals);
    glGenBuffers(1, &m_ebo);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &m_vboPositions);
    glDeleteBuffers(1, &m_vboNormals);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}

void Mesh::SetVertexPositions(const float* positions, std::size_t count)
{
    m_vertexCount = count;
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
    glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // layout(location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::SetVertexNormals(const float* normals, std::size_t count)
{
    // assume same vertex count
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboNormals);
    glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1); // layout(location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::SetIndices(const void* indices, std::size_t count, int componentType)
{
    m_indexCount = count;
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        count * (componentType == GL_UNSIGNED_SHORT ? sizeof(unsigned short) : sizeof(unsigned int)),
        indices,
        GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void Mesh::Draw() const
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_indexCount,
        (m_indexCount > 0 && m_indexCount * sizeof(unsigned int) / m_indexCount > 2)
            ? GL_UNSIGNED_INT
            : GL_UNSIGNED_SHORT,
        nullptr);
    glBindVertexArray(0);
}
