#include "core/Scene.hpp"
#include <switch/services/hid.h>
#include "input/InputSystem.hpp"
#include <switch.h>

int main(int argc, char** argv)
{
    consoleInit(nullptr);
    Scene scene;
    InputSystem input;

    u64 start = armGetSystemTick();

    while (appletMainLoop()) {
        // Δt in seconds
        u64 now = armGetSystemTick();
        float dt = (now - start) / static_cast<float>(armGetSystemTickFreq());
        start = now;

        input.Pump();

        if (input.WasJustPressed(KEY_PLUS))
            break; // exit

        scene.Update(dt);

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
