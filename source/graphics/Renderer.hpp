#pragma once

#include <glm/glm.hpp>

void gfxInit(); // call once at start
void gfxBegin(); // clear + begin frame
void gfxEnd(); // swap buffers
void gfxExit(); // cleanup
void updateViewProj(const glm::mat4& view, const glm::mat4& proj);
