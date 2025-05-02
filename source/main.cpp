// source/main.cpp

//======================================
// Core systems
//======================================
#include <memory>
#include <string>
#include <vector>

//======================================
// Graphics APIs
//======================================
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <switch.h>

//======================================
// Engine components
//======================================
#include "Player.hpp"
#include "components/AnimationComponent.hpp"
#include "components/GltfComponent.hpp"
#include "core/Camera.hpp"
#include "core/Component.hpp"
#include "core/GameObject.hpp"
#include "core/Logging.hpp"
#include "core/Scene.hpp"
#include "core/Transform.hpp"
#include "graphics/Renderer.hpp"
#include "input/InputSystem.hpp"
#include "renderer/Mesh.hpp"

class Engine {
public:
    static void Init()
    {
        initLogging();
        romfsInit();
        gfxInit();

        if (!gladLoadGL()) {
            LOG_ERROR("Failed to initialize GLAD");
            return;
        }

        glEnable(GL_DEPTH_TEST);
        LOG_INFO("Engine initialized successfully");
    }

    static void Shutdown()
    {
        gfxExit();
        romfsExit();
        LoggingExit();
        LOG_INFO("Engine shutdown complete");
    }

    static float CalculateDeltaTime(u64& prevTime)
    {
        const double freq = static_cast<double>(armGetSystemTickFreq());
        u64 now = armGetSystemTick();
        float dt = static_cast<float>((now - prevTime) / freq);
        prevTime = now;
        return dt;
    }
};

class Game {
public:
    Game()
        : m_input()
        , m_scene()
    {
        SetupScene();
    }

    void Run()
    {
        u64 prevTime = armGetSystemTick();

        while (appletMainLoop()) {
            const float dt = Engine::CalculateDeltaTime(prevTime);
            Update(dt);
            Render();

            if (ShouldExit())
                break;
        }
    }

private:
    void SetupScene()
    {
        // Setup player camera
        auto& playerCam = m_scene.root().createChild("PlayerCamera");
        playerCam.transform().position = { 0.0f, 0.0f, 20.0f }; // Move camera back appropriately
        playerCam.addComponent<Player>(&playerCam, &m_input);
        m_mainCamera = &playerCam.addComponent<Camera>(
            &playerCam, 78.0f, 1280.0f / 720.0f, 0.1f, 100.0f); // Adjust far plane as needed

        auto animationClips = Asset::GltfLoader::LoadAnimations("romfs:/GLBs/girl.glb");

        std::shared_ptr<AnimationClip> walkingClip = nullptr;
        for (auto& clip : animationClips) {
            if (clip->GetName() == "walking") {
                walkingClip = clip;
                break;
            }
        }

        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                auto& girlObj = m_scene.root().createChild("Girl" + std::to_string(i * 5 + j));
                girlObj.transform().position = { float(i) * 1.0f, 0.0f, float(j) * 3.0f };
                girlObj.addComponent<GltfComponent>(&girlObj, "romfs:/GLBs/girl.glb");

                if (walkingClip) {
                    girlObj.addComponent<AnimationComponent>(&girlObj, walkingClip);
                }
            }
        }
    }

    void Update(float dt)
    {
        m_input.update();
        m_scene.Update(dt);
        updateViewProj(m_mainCamera->viewMatrix(),
            m_mainCamera->projectionMatrix());
    }

    void Render()
    {
        gfxBegin();
        {
            glClearColor(0.0f, 0.2f, 0.6f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, 1280, 720);

            m_scene.Render();
        }
        gfxEnd();
    }

    bool ShouldExit() const
    {
        return m_input.keysDown() & HidNpadButton_Plus;
    }

    InputSystem m_input;
    Scene m_scene;
    Camera* m_mainCamera = nullptr;
};

int main(int argc, char** argv)
{
    Engine::Init();

    Game game;
    game.Run();

    Engine::Shutdown();
    return 0;
}
