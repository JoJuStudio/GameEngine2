// source/graphics/GLUtils.hpp
#pragma once
#include "core/Logging.hpp"
#include <glad/glad.h>
#include <string>

namespace GLUtils {

/// Compile a shader of the given type. On failure, log via LOG_ERROR and
/// optionally write the compiler log into logOut.
GLuint compileShader(GLenum type, const char* src, std::string* logOut = nullptr);

/// Link a vertex+fragment program. On failure, log via LOG_ERROR and
/// optionally write the link log into logOut.
GLuint linkProgram(GLuint vs, GLuint fs, std::string* logOut = nullptr);

/// Check for a GL error; logs one if found.
inline bool checkError(const char* label = nullptr)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        if (label)
            LOG_ERROR("GL error @ %s: 0x%04X", label, err);
        else
            LOG_ERROR("GL error: 0x%04X", err);
        return false;
    }
    return true;
}

} // namespace GLUtils
