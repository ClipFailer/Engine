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
