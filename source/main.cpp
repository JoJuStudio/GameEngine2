// source/main.cpp

#include <glad/glad.h>
#include <switch.h>

#include "Player.hpp" // your movement/look component
#include "asset/GltfLoader.hpp" // Asset::GltfLoader
#include "core/Camera.hpp" // Camera component
#include "core/GameObject.hpp" // full definition of GameObject
#include "core/Logging.hpp" // initLogging(), LOG_*, LoggingExit()
#include "core/Scene.hpp" // Scene, root()
#include "graphics/Renderer.hpp" // gfxInit, gfxBegin/End, updateViewProj
#include "input/InputSystem.hpp" // InputSystem
#include "renderer/Mesh.hpp" // Mesh::Draw()

// A tiny Component that holds shared_ptr<Mesh> so it matches GltfLoader
struct GltfComponent : public Component {
    std::vector<std::shared_ptr<Mesh>> meshes;

    GltfComponent(GameObject* owner, const std::string& path)
        : Component(owner)
    {
        auto assetScene = Asset::GltfLoader::LoadFromFile(path);
        meshes = assetScene.meshes; // copy the shared_ptrs
    }

    // your engine will call this during its render pass
    void OnDraw() const override
    {
        for (auto& m : meshes)
            m->Draw();
    }
};

int main(int, char**)
{
    // — logging to console/file —
    initLogging();
    consoleInit(NULL); // also route printf/LOG_* to the screen

    // — mount ROMFS —
    romfsInit();

    // — bring up the GPU —
    gfxInit();

    // — load GL pointers & depth test —
    if (!gladLoadGL()) {
        LOG_ERROR("gladLoadGL failed");
    }
    glEnable(GL_DEPTH_TEST);

    // — set up input + scene + player+camera —
    InputSystem input;
    Scene scene;

    auto& playerCam = scene.root().createChild("PlayerCamera");
    playerCam.transform().position = { 0, 0, 3 };
    playerCam.addComponent<Player>(&playerCam, &input);
    auto& cam = playerCam.addComponent<Camera>(
        &playerCam, 78.f, 1280.f / 720.f, 0.1f, 100.f);

    // — load & attach the girl model —
    auto& girlObj = scene.root().createChild("Girl");
    girlObj.transform().position = { 0, 0, 0 };
    girlObj.addComponent<GltfComponent>(&girlObj, "romfs:/GLBs/girl.glb");

    // — main loop —
    const double freq = double(armGetSystemTickFreq());
    u64 prev = armGetSystemTick();
    while (appletMainLoop()) {
        u64 now = armGetSystemTick();
        float dt = float((now - prev) / freq);
        prev = now;

        input.update();
        if (input.keysDown() & HidNpadButton_Plus)
            break;

        scene.Update(dt);
        updateViewProj(cam.viewMatrix(), cam.projectionMatrix());

        gfxBegin();
        scene.Render(); // <-- replace with your engine’s actual render call
        gfxEnd();
    }

    // — tear down —
    gfxExit();
    romfsExit();
    LoggingExit();
    return 0;
}
