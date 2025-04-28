#pragma once

#include <glad/glad.h>
#include <string>

namespace Graphics {

class ShaderLoader {
public:
    // Load a vertex + fragment shader from ROMFS and link into a program
    static GLuint LoadProgram(const std::string& vertexPath, const std::string& fragmentPath);

private:
    static std::string LoadFile(const std::string& path);
    static GLuint CompileShader(GLenum type, const std::string& source);
};

} // namespace Graphics
