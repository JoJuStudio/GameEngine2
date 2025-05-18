// source/main.cpp

#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <switch.h>

#include "components/AnimationComponent.hpp"
#include "components/GltfComponent.hpp"
#include "core/Camera.hpp"
#include "core/Transform.hpp"
#include "core/GameObject.hpp"
#include "core/Logging.hpp"
#include "core/Scene.hpp"
#include "graphics/Renderer.hpp"
#include "input/InputSystem.hpp"
#include "Player.hpp"

static u64 previousTick;
static Camera* mainCamera = nullptr;
static Scene scene;
static InputSystem inputSystem;

void initEngine() {
    initLogging();
    Logger::DisableFileLogging();
    romfsInit();
    gfxInit();

    if (!gladLoadGL()) {
        LOG_ERROR("Failed to initialize GLAD");
        exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
    previousTick = armGetSystemTick();
    LOG_INFO("Engine initialized");
}

void shutdownEngine() {
    gfxExit();
    romfsExit();
    LoggingExit();
    LOG_INFO("Engine shutdown");
}

float calculateDeltaTime() {
    double freq = static_cast<double>(armGetSystemTickFreq());
    u64 currentTick = armGetSystemTick();
    float dt = static_cast<float>((currentTick - previousTick) / freq);
    previousTick = currentTick;
    return dt;
}

void buildScene() {
    auto& root = scene.root();
    auto& playerCam = root.createChild("PlayerCamera");
    playerCam.transform().position = {0.0f, 0.0f, 20.0f};

    playerCam.addComponent<Player>(&playerCam, &inputSystem);
    mainCamera = &playerCam.addComponent<Camera>(&playerCam, 78.0f, 1280.0f/720.0f, 0.1f, 100.0f);

    const std::string glbPath = "romfs:/GLBs/girl.glb";
    auto animations = Asset::GltfLoader::LoadAnimations(glbPath);
    auto walkingClip = std::find_if(animations.begin(), animations.end(),
        [](auto& clip){ return clip->GetName() == "walking"; });

    auto& girlObj = root.createChild("Girl0");
    girlObj.transform().position = {0.0f, 0.0f, 0.0f};
    girlObj.addComponent<GltfComponent>(&girlObj, glbPath);
    if (walkingClip != animations.end())
        girlObj.addComponent<AnimationComponent>(&girlObj, *walkingClip);

    LOG_INFO("Scene built");
}

void update(float dt) {
    inputSystem.update();
    scene.Update(dt);
    updateViewProj(mainCamera->viewMatrix(), mainCamera->projectionMatrix());
}

void render() {
    gfxBegin();
    glClearColor(0.0f, 0.2f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, 1280, 720);
    scene.Render();
    gfxEnd();
}

int main() {
    initEngine();
    buildScene();

    while (appletMainLoop()) {
        float dt = calculateDeltaTime();
        update(dt);
        render();
        if (inputSystem.keysDown() & HidNpadButton_Plus) break;
    }

    shutdownEngine();
    return EXIT_SUCCESS;
}
