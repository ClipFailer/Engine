/**
 * @file main.cpp
 * @brief Entry point for the Vulkan 3D Game & Simulation Engine Editor.
 */

#include "editor/EditorApp.hpp"
#include "core/Log.hpp"
#include "scene/Component.hpp"
#include "scene/Actor.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "scene/AudioComponent.hpp"
#include "scene/PositionComponent.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/MovementComponent.hpp"
#include "animation/SkeletalMeshComponent.hpp"

/**
 * @brief Main engine application entry point.
 * @param argc Argument count.
 * @param argv Argument vectors.
 * @return Process exit code.
 */
int main(int argc, char* argv[]) {
    // 1. Register Engine Reflection metadata
    Engine::Component::RegisterReflection();
    Engine::Actor::RegisterReflection();
    Engine::StaticMeshComponent::RegisterReflection();
    Engine::LightComponent::RegisterReflection();
    Engine::RigidBodyComponent::RegisterReflection();
    Engine::AudioComponent::RegisterReflection();
    Engine::PositionComponent::RegisterReflection();
    Engine::PhysicsComponent::RegisterReflection();
    Engine::CollisionComponent::RegisterReflection();
    Engine::MovementComponent::RegisterReflection();
    Engine::SkeletalMeshComponent::RegisterReflection();

    LOG_INFO("Vulkan 3D Engine Editor starting up...");

    // 2. Instantiate and launch Editor
    try {
        Engine::EditorApp app;
        if (!app.Initialize()) {
            LOG_ERROR("Engine Editor failed to initialize");
            return -1;
        }

        LOG_INFO("Entering engine main loop");
        app.Run();
        LOG_INFO("Engine Editor exited successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("Unhandled engine exception: {}", e.what());
        return -1;
    }

    return 0;
}

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/**
 * @brief Windows WinMain entry point for GUI subsystem without an OS console window.
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    FreeConsole();
    return main(__argc, __argv);
}
#endif
