// source/main.cpp

#include <glad/glad.h>
#include <switch.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Player.hpp" // your movement/look component
#include "asset/GltfLoader.hpp" // Asset::GltfLoader
#include "core/Camera.hpp" // Camera component
#include "core/Component.hpp" // Component base class
#include "core/GameObject.hpp" // GameObject
#include "core/Logging.hpp" // initLogging(), LOG_*, LoggingExit()
#include "core/Scene.hpp" // Scene, root(), Update(), Render()
#include "core/Transform.hpp" // Transform
#include "graphics/Renderer.hpp" // gfxInit/Exit, gfxBegin/End, updateViewProj
#include "input/InputSystem.hpp" // InputSystem
#include "renderer/Mesh.hpp" // Mesh::Draw()

#include <memory>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
// A tiny Component that just holds and draws all the meshes loaded from a glTF.
//------------------------------------------------------------------------------
struct GltfComponent : public Component {
    std::vector<std::shared_ptr<Mesh>> meshes;

    GltfComponent(GameObject* owner, const std::string& path)
        : Component(owner)
    {
        auto assetScene = Asset::GltfLoader::LoadFromFile(path);
        meshes = assetScene.meshes;

        if (meshes.empty()) {
            LOG_ERROR("GltfComponent: failed to load meshes from '%s'",
                path.c_str());
        } else {
            LOG_INFO("GltfComponent: loaded %zu mesh(es) from '%s'",
                meshes.size(), path.c_str());
        }
    }

    ComponentTypeID type() const override
    {
        return componentTypeID<GltfComponent>();
    }

    void draw() override
    {
        // each frame, just draw all meshes (using whatever shader they baked in)
        for (auto& mesh : meshes) {
            mesh->Draw();
        }
    }
};

//------------------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------------------
int main(int, char**)
{
    // — logging & on-screen console —
    initLogging();

    // — mount ROMFS & init GPU —
    romfsInit();
    gfxInit();

    // — load GL pointers & enable depth test —
    if (!gladLoadGL()) {
        LOG_ERROR("gladLoadGL failed");
    }
    glEnable(GL_DEPTH_TEST);

    // — set up input, scene, player+camera —
    InputSystem input;
    Scene scene;

    auto& playerCam = scene.root().createChild("PlayerCamera");
    playerCam.transform().position = { 0.0f, 0.0f, 3.0f };
    playerCam.addComponent<Player>(&playerCam, &input);
    auto& cam = playerCam.addComponent<Camera>(
        &playerCam,
        /*fov=*/78.f,
        /*aspect=*/1280.f / 720.f,
        /*znear=*/0.1f,
        /*zfar=*/100.f);

    // — load & attach the girl model at world-origin —
    auto& girlObj = scene.root().createChild("Girl");
    girlObj.transform().position = { 0.0f, 0.0f, 0.0f };
    girlObj.addComponent<GltfComponent>(&girlObj,
        "romfs:/GLBs/girl.glb");

    // — timing setup —
    const double freq = double(armGetSystemTickFreq());
    u64 prev = armGetSystemTick();

    // — main loop —
    while (appletMainLoop()) {
        // compute delta‐time
        u64 now = armGetSystemTick();
        float dt = float((now - prev) / freq);
        prev = now;

        // poll input & exit if “+” pressed
        input.update();
        if (input.keysDown() & HidNpadButton_Plus)
            break;

        // update scene graph
        scene.Update(dt);

        // push camera matrices
        updateViewProj(cam.viewMatrix(), cam.projectionMatrix());

        // start frame
        gfxBegin();

        // clear to a nice sky‐blue
        glClearColor(0.0f, 0.2f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 1280, 720);

        // draw everything
        scene.Render();

        // finish frame
        gfxEnd();
    }

    // — tear everything down —
    gfxExit();
    romfsExit();
    LoggingExit();

    return 0;
}
