#include <EGL/egl.h>
#include <glad/glad.h>
#include <switch.h>

#include "GLUtils.hpp"
#include "core/Logging.hpp"

// raw GLSL sources
static const char* vertexSrc = R"text(
    #version 300 es
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    out vec3 ourColor;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor;
    }
)text";

static const char* fragmentSrc = R"text(
    #version 300 es
    precision mediump float;
    in vec3 ourColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(ourColor, 1.0);
    }
)text";

/* EGL + GL handles */
static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLContext s_context = EGL_NO_CONTEXT;
static EGLSurface s_surface = EGL_NO_SURFACE;

/* Triangle program + buffer */
static GLuint s_prog = 0;
static GLuint s_vbo = 0;

void gfxInit()
{
    // 1. EGL setup
    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(s_display, nullptr, nullptr);

    const EGLint cfg_attribs[] = {
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
    EGLConfig cfg;
    EGLint n;
    eglChooseConfig(s_display, cfg_attribs, &cfg, 1, &n);

    NWindow* win = nwindowGetDefault();
    nwindowSetSwapInterval(win, 1);
    s_surface = eglCreateWindowSurface(s_display, cfg, win, nullptr);

    const EGLint ctx_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    s_context = eglCreateContext(s_display, cfg, EGL_NO_CONTEXT, ctx_attribs);
    eglMakeCurrent(s_display, s_surface, s_surface, s_context);

    // 2. Load GL functions
    if (!gladLoadGLLoader((GLADloadproc)eglGetProcAddress)) {
        LOG_ERROR("Failed to load GL via glad");
    } else {
        LOG_INFO("Glad loaded");
    }

    // 3. Upload interleaved triangle data: pos.xyz + color.rgb
    const GLfloat verts[] = {
        //    x      y     z      r    g    b
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // red
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // green
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f // blue
    };
    glGenBuffers(1, &s_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // 4. Compile & link shaders
    GLuint vs = GLUtils::compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = GLUtils::compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    s_prog = GLUtils::linkProgram(vs, fs);

    LOG_INFO("Shader program %u created", s_prog);
}

void gfxBegin()
{
    glViewport(0, 0, 1280, 720);
    glClearColor(0.1f, 0.1f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw colored triangle
    glUseProgram(s_prog);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);

    // position @ location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(GLfloat),
        (void*)0);

    // color @ location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(GLfloat),
        (void*)(3 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    GLUtils::checkError("drawColoredTriangle");
}

void gfxEnd()
{
    eglSwapBuffers(s_display, s_surface);
}

void gfxExit()
{
    glDeleteBuffers(1, &s_vbo);
    glDeleteProgram(s_prog);

    eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(s_display, s_context);
    eglDestroySurface(s_display, s_surface);
    eglTerminate(s_display);

    LOG_INFO("Renderer shutdown");
}
