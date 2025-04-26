#include "core/Scene.hpp"
#include "input/InputSystem.hpp"
#include <switch.h>

int main(int argc, char** argv)
{
    consoleInit(nullptr);

    Scene scene;
    InputSystem input;

    u64 prev = armGetSystemTick();
    const double freq = static_cast<double>(armGetSystemTickFreq());

    while (appletMainLoop()) {
        u64 now = armGetSystemTick();
        float dt = static_cast<float>((now - prev) / freq);
        prev = now;

        input.update();

        if (input.keysDown() & HidNpadButton_Plus)
            break;

        scene.Update(dt);
        consoleUpdate(nullptr);
    }

    consoleExit(nullptr);
    return 0;
}
