// source/graphics/Renderer.cpp

#include "graphics/Renderer.hpp"
#include "graphics/GLUtils.hpp"
#include "graphics/ShaderLoader.hpp"

#include <EGL/egl.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <switch.h>

// -- EGL/GL state --
static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLContext s_context = EGL_NO_CONTEXT;
static EGLSurface s_surface = EGL_NO_SURFACE;

// -- GL program & uniform locations --
static GLuint s_prog = 0;
static GLint s_viewLoc = -1;
static GLint s_projLoc = -1;
static GLint s_modelLoc = -1;
static GLint s_textureLoc = -1;

// -- Stored view/proj matrices --
static glm::mat4 s_view = glm::mat4(1.0f);
static glm::mat4 s_proj = glm::mat4(1.0f);

void gfxInit()
{
    // EGL + GL context setup
    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(s_display, nullptr, nullptr);

    const EGLint fb_attr[] = {
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
    EGLConfig cfg;
    EGLint n;
    eglChooseConfig(s_display, fb_attr, &cfg, 1, &n);

    NWindow* win = nwindowGetDefault();
    nwindowSetSwapInterval(win, 1);
    s_surface = eglCreateWindowSurface(s_display, cfg, win, nullptr);

    const EGLint ctx_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    s_context = eglCreateContext(s_display, cfg, EGL_NO_CONTEXT, ctx_attr);
    eglMakeCurrent(s_display, s_surface, s_surface, s_context);

    gladLoadGLLoader((GLADloadproc)eglGetProcAddress);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Compile and link shaders from romfs
    s_prog = Graphics::ShaderLoader::LoadProgram(
        "romfs:/Shaders/vertex.glsl",
        "romfs:/Shaders/fragment.glsl");

    // Locate uniforms
    s_viewLoc = glGetUniformLocation(s_prog, "uView");
    s_projLoc = glGetUniformLocation(s_prog, "uProj");
    s_modelLoc = glGetUniformLocation(s_prog, "uModel");
    s_textureLoc = glGetUniformLocation(s_prog, "uTexture");
}

void updateViewProj(const glm::mat4& view, const glm::mat4& proj)
{
    s_view = view;
    s_proj = proj;
}

void setModelMatrix(const glm::mat4& model)
{
    if (s_modelLoc != -1) {
        glUniformMatrix4fv(s_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }
}

void bindDefaultTexture(GLuint textureID)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    if (s_textureLoc != -1) {
        glUniform1i(s_textureLoc, 0); // Texture unit 0
    }
}

void gfxBegin()
{
    glViewport(0, 0, 1280, 720);
    glClearColor(0.1f, 0.1f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(s_prog);

    glUniformMatrix4fv(s_viewLoc, 1, GL_FALSE, glm::value_ptr(s_view));
    glUniformMatrix4fv(s_projLoc, 1, GL_FALSE, glm::value_ptr(s_proj));
}

void gfxEnd()
{
    eglSwapBuffers(s_display, s_surface);
}

void gfxExit()
{
    glDeleteProgram(s_prog);

    eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(s_display, s_context);
    eglDestroySurface(s_display, s_surface);
    eglTerminate(s_display);
}
