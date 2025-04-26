// source/graphics/Renderer.cpp
#include "graphics/Renderer.hpp"
#include "graphics/GLUtils.hpp"

#include <EGL/egl.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <switch.h>
#include <vector>

// -- GLSL shaders, matching simple_triangle example but with view/proj --
static const char* const vertexShaderSource = R"text(
    attribute vec3 aPos;
    attribute vec3 aColor;
    uniform mat4 uView;
    uniform mat4 uProj;
    varying   vec3 vColor;
    void main() {
        gl_Position = uProj * uView * vec4(aPos, 1.0);
        vColor = aColor;
    }
)text";

static const char* const fragmentShaderSource = R"text(
    precision mediump float;
    varying vec3 vColor;
    void main() {
        gl_FragColor = vec4(vColor, 1.0);
    }
)text";

// -- Base cube positions: 36 vertices × xyz --
static const GLfloat s_cubePositions[36 * 3] = {
    // front face
    -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
    // back face
    -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
    // left face
    -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
    // right face
    0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,
    // top face
    -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
    // bottom face
    -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f
};

// -- EGL/GL state --
static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLContext s_context = EGL_NO_CONTEXT;
static EGLSurface s_surface = EGL_NO_SURFACE;

// -- GL objects & locations --
static GLuint s_prog = 0;
static GLuint s_vbo = 0;
static GLint s_posLoc = -1;
static GLint s_colorLoc = -1;
static GLint s_viewLoc = -1;
static GLint s_projLoc = -1;

// -- Stored view/proj matrices --
static glm::mat4 s_view = glm::mat4(1.0f);
static glm::mat4 s_proj = glm::mat4(1.0f);

void gfxInit()
{
    // 1) EGL + GL context
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

    // 2) Build interleaved VBO: position + color
    std::vector<GLfloat> data;
    data.reserve(36 * 6);
    for (int i = 0; i < 36; ++i) {
        float x = s_cubePositions[i * 3 + 0];
        float y = s_cubePositions[i * 3 + 1];
        float z = s_cubePositions[i * 3 + 2];
        // map [-0.5,0.5]→[0,1] for RGB
        float r = x + 0.5f;
        float g = y + 0.5f;
        float b = z + 0.5f;
        data.push_back(x);
        data.push_back(y);
        data.push_back(z);
        data.push_back(r);
        data.push_back(g);
        data.push_back(b);
    }
    glGenBuffers(1, &s_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat),
        data.data(), GL_STATIC_DRAW);

    // 3) Compile & link shaders
    GLuint vs = GLUtils::compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = GLUtils::compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    s_prog = GLUtils::linkProgram(vs, fs);

    // 4) Locate attributes & uniforms
    s_posLoc = glGetAttribLocation(s_prog, "aPos");
    s_colorLoc = glGetAttribLocation(s_prog, "aColor");
    s_viewLoc = glGetUniformLocation(s_prog, "uView");
    s_projLoc = glGetUniformLocation(s_prog, "uProj");
}

void updateViewProj(const glm::mat4& view,
    const glm::mat4& proj)
{
    s_view = view;
    s_proj = proj;
}

void gfxBegin()
{
    glViewport(0, 0, 1280, 720);
    glClearColor(0.1f, 0.1f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(s_prog);
    glUniformMatrix4fv(s_viewLoc, 1, GL_FALSE, glm::value_ptr(s_view));
    glUniformMatrix4fv(s_projLoc, 1, GL_FALSE, glm::value_ptr(s_proj));

    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);

    glEnableVertexAttribArray(s_posLoc);
    glVertexAttribPointer(s_posLoc, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(s_colorLoc);
    glVertexAttribPointer(s_colorLoc, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(GLfloat),
        (void*)(3 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisableVertexAttribArray(s_posLoc);
    glDisableVertexAttribArray(s_colorLoc);
}

void gfxEnd()
{
    eglSwapBuffers(s_display, s_surface);
}

void gfxExit()
{
    glDeleteBuffers(1, &s_vbo);
    glDeleteProgram(s_prog);

    eglMakeCurrent(s_display, EGL_NO_SURFACE,
        EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(s_display, s_context);
    eglDestroySurface(s_display, s_surface);
    eglTerminate(s_display);
}
