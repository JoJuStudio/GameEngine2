// source/graphics/GLUtils.cpp
#include "GLUtils.hpp"
#include <vector>

namespace GLUtils {

static void fetchLog(GLuint obj, bool isShader, std::string* out)
{
    if (!out)
        return;
    GLint len = 0;
    if (isShader)
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
    else
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
    std::vector<char> buf(len);
    if (isShader)
        glGetShaderInfoLog(obj, len, nullptr, buf.data());
    else
        glGetProgramInfoLog(obj, len, nullptr, buf.data());
    *out = std::string(buf.data(), buf.data() + len);
}

GLuint compileShader(GLenum type, const char* src, std::string* logOut)
{
    GLuint obj = glCreateShader(type);
    glShaderSource(obj, 1, &src, nullptr);
    glCompileShader(obj);

    GLint status = GL_FALSE;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        fetchLog(obj, /*isShader=*/true, logOut);
        LOG_ERROR("Shader compile failed (type %u): %s",
            unsigned(type),
            logOut ? logOut->c_str() : "no log");
    }
    return obj;
}

GLuint linkProgram(GLuint vs, GLuint fs, std::string* logOut)
{
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint status = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        fetchLog(prog, /*isShader=*/false, logOut);
        LOG_ERROR("Program link failed: %s",
            logOut ? logOut->c_str() : "no log");
    }
    return prog;
}

} // namespace GLUtils
