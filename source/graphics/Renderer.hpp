// source/graphics/Renderer.hpp

#pragma once

#include <glm/glm.hpp>

// Core graphics functions
void gfxInit(); // Initialize EGL, GL, shaders
void gfxExit(); // Shutdown EGL, GL
void gfxBegin(); // Start a new frame (clear, bind shader)
void gfxEnd();   // End frame (swap buffers)

// VSync control
void gfxSetVsyncMode(bool enable);

// Matrix management
void updateViewProj(const glm::mat4& view, const glm::mat4& proj);
void setModelMatrix(const glm::mat4& model);
