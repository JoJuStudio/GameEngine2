// source/main.cpp
#include <switch.h>

#include "Player.hpp"
#include "core/Camera.hpp"
#include "core/GameObject.hpp"
#include "core/Logging.hpp" // initLogging(), LoggingExit()
#include "core/Scene.hpp"
#include "core/Transform.hpp"
#include "graphics/Renderer.hpp"
#include "input/InputSystem.hpp"

int main(int, char**)
{
    initLogging();
    gfxInit();

    InputSystem input;
    Scene scene;

    // 1) spawn a single object that is both player & camera
    auto& obj = scene.root().createChild("PlayerCamera");
    obj.transform().position = { 0.f, 0.f, 3.f };

    // 2) add your movement/look component
    obj.addComponent<Player>(&obj, &input);

    // 3) add the camera component
    auto& cam = obj.addComponent<Camera>(
        &obj,
        78.f, // FOV
        1280.f / 720.f, // aspect ratio
        0.1f, // near plane
        100.f // far plane
    );

    const double freq = double(armGetSystemTickFreq());
    u64 prev = armGetSystemTick();

    while (appletMainLoop()) {
        // timestep
        u64 now = armGetSystemTick();
        float dt = float((now - prev) / freq);
        prev = now;

        // input & exit
        input.update();
        if (input.keysDown() & HidNpadButton_Plus)
            break;

        // update components
        scene.Update(dt);

        // push camera matrices to renderer
        updateViewProj(cam.viewMatrix(), cam.projectionMatrix());

        // draw
        gfxBegin();
        gfxEnd();
    }

    LoggingExit();
    gfxExit();
    return 0;
}
