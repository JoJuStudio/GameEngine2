// source/graphics/Renderer.cpp
#include <EGL/egl.h>
#include <glad/glad.h> // <- generic header
#include <switch.h>

static EGLDisplay s_display;
static EGLContext s_context;
static EGLSurface s_surface;

void gfxInit()
{
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

    /* generic loader for glad/glad.h */
    gladLoadGLLoader((GLADloadproc)eglGetProcAddress);
}

void gfxBegin()
{
    glViewport(0, 0, 1280, 720);
    glClearColor(0.1f, 0.1f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gfxEnd() { eglSwapBuffers(s_display, s_surface); }

void gfxExit()
{
    eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(s_display, s_context);
    eglDestroySurface(s_display, s_surface);
    eglTerminate(s_display);
}
