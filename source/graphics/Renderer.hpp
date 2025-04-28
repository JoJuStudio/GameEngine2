// source/graphics/Renderer.hpp

#pragma once

#include <glm/glm.hpp>

void gfxInit(); // Initialize EGL, GL, shaders
void gfxExit(); // Shutdown EGL, GL
void gfxBegin(); // Start a new frame (clear, bind shader)
void gfxEnd(); // End frame (swap buffers)

void updateViewProj(const glm::mat4& view, const glm::mat4& proj); // Set View and Projection matrices
void setModelMatrix(const glm::mat4& model); // Set Model matrix per-object
