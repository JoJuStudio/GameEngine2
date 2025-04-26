#include "core/Logging.hpp"
#include "core/Scene.hpp"
#include "graphics/Renderer.hpp"
#include "input/InputSystem.hpp"
#include <switch.h>

int main(int, char**)
{
    gfxInit();
    InputSystem input;
    Scene scene;
    initLogging();

    const double freq = static_cast<double>(armGetSystemTickFreq());
    u64 prev = armGetSystemTick();

    while (appletMainLoop()) {
        u64 now = armGetSystemTick();
        float dt = static_cast<float>((now - prev) / freq);
        prev = now;

        input.update();
        if (input.keysDown() & HidNpadButton_Plus)
            break;

        scene.Update(dt);

        gfxBegin();
        /* draw scene here */
        gfxEnd();
    }

    LoggingExit();
    gfxExit();
    return 0;
}
