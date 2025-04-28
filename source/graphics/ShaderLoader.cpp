#include "ShaderLoader.hpp"
#include "../core/Logging.hpp"

#include <fstream>
#include <sstream>
#include <vector>

namespace Graphics {

std::string ShaderLoader::LoadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        LOG_ERROR("ShaderLoader: Failed to open %s", path.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint ShaderLoader::CompileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check for compile errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOG_ERROR("ShaderLoader: Shader compile error: %s", infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint ShaderLoader::LoadProgram(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexCode = LoadFile(vertexPath);
    std::string fragmentCode = LoadFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty()) {
        LOG_ERROR("ShaderLoader: Failed to load shader sources.");
        return 0;
    }

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentCode);

    if (vertexShader == 0 || fragmentShader == 0) {
        LOG_ERROR("ShaderLoader: Failed to compile shaders.");
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check link status
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        LOG_ERROR("ShaderLoader: Program link error: %s", infoLog);
        glDeleteProgram(program);
        return 0;
    }

    // Cleanup shaders (linked now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOG_INFO("ShaderLoader: Successfully loaded program (%s + %s)", vertexPath.c_str(), fragmentPath.c_str());
    return program;
}

} // namespace Graphics
