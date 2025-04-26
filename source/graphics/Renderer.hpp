#pragma once

void gfxInit(); // call once at start
void gfxBegin(); // clear + begin frame
void gfxEnd(); // swap buffers
void gfxExit(); // cleanup
