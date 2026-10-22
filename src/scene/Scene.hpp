/**
 * @file Scene.hpp
 * @brief Scene container managing actors, levels, serialization, and simulation state.
 */

#pragma once

#include "Actor.hpp"
#include "LightComponent.hpp"
#include "core/Types.hpp"
#include <vector>
#include <memory>
#include <string>

namespace Engine {

    /**
     * @struct ActorSnapshot
     * @brief Stored state before Play mode to restore transforms and states on Stop.
     */
    struct ActorSnapshot {
        UUID Id;
        Transform Transform;
    };

    /**
     * @class Scene
     * @brief World container managing all actors, physics simulation, and level lifecycles.
     */
    class Scene {
    public:
        Scene();
        ~Scene();

        /**
         * @brief Spawns a new actor of type T in the scene.
         * @tparam T Actor class type.
         * @tparam Args Variadic argument pack type for constructor.
         * @param name Optional display name.
         * @param args Constructor arguments passed to actor.
         * @return Pointer to spawned actor.
         */
        template<typename T = Actor, typename... Args>
        T* SpawnActor(std::string name = "Actor", Args&&... args) {
            auto actor = std::make_unique<T>(std::move(name), std::forward<Args>(args)...);
            T* ptr = actor.get();
            m_Actors.push_back(std::move(actor));
            return ptr;
        }

        /**
         * @brief Spawns a lightweight light actor (Directional or Point) without physics, mesh, or collision.
         * @param name Name of the light entity.
         * @param type Light type (Directional or Point).
         * @return Pointer to spawned light actor.
         */
        Actor* SpawnLightActor(const std::string& name, LightType type);

        /**
         * @brief Clears all actors from the level creating an empty scene.
         */
        void ClearAllActors() {
            if (OnActorsCleared) OnActorsCleared();
            m_Actors.clear();
        }

        std::function<void()> OnActorsCleared;

        /**
         * @brief Destroys and removes an actor from the scene.
         * @param actor Actor to destroy.
         */
        void DestroyActor(Actor* actor);

        /**
         * @brief Finds actor by its unique UUID.
         * @param id 64-bit UUID.
         * @return Pointer to Actor or nullptr.
         */
        [[nodiscard]] Actor* FindActorByUUID(UUID id) const;

        /**
         * @brief Returns collection of all actors in the level.
         * @return Vector of unique pointers to actors.
         */
        [[nodiscard]] const std::vector<std::unique_ptr<Actor>>& GetActors() const noexcept {
            return m_Actors;
        }

        /**
         * @brief Current simulation state.
         * @return Stopped, Playing, or Paused.
         */
        [[nodiscard]] SimulationState GetState() const noexcept { return m_State; }

        /**
         * @brief Starts Play in Editor (PIE) simulation.
         */
        void StartSimulation();

        /**
         * @brief Ticks the simulation step for all active actors.
         * @param deltaTime Seconds elapsed.
         */
        void Tick(float deltaTime);

        /**
         * @brief Stops simulation and restores actors to initial positions.
         */
        void StopSimulation();

        /**
         * @brief Pauses active simulation.
         */
        void PauseSimulation() { m_State = SimulationState::Paused; }

        /**
         * @brief Resumes paused simulation.
         */
        void ResumeSimulation() { m_State = SimulationState::Playing; }

        /**
         * @brief Populates default scene with ground plane, directional light, cube, and sphere.
         */
        void SetupDefaultScene();

        /**
         * @brief Spawns a new Actor into the scene instantiated from a .actor asset blueprint.
         * @param assetPath Path to the .actor file.
         * @param spawnPosition World position to place the spawned actor.
         * @return Pointer to spawned Actor.
         */
        Actor* SpawnActorFromAsset(const std::filesystem::path& assetPath, const glm::vec3& spawnPosition = {0.0f, 1.0f, 0.0f});

        /**
         * @brief Serializes scene to JSON file.
         * @param filepath Destination path.
         * @return True if saved successfully.
         */
        bool SaveToFile(const std::string& filepath);

        /**
         * @brief Loads scene from JSON file.
         * @param filepath Source file path.
         * @return True if loaded successfully.
         */
        bool LoadFromFile(const std::string& filepath);

    private:
        SimulationState m_State{SimulationState::Stopped};
        std::vector<std::unique_ptr<Actor>> m_Actors;
        std::vector<ActorSnapshot> m_InitialSnapshots;
    };

} // namespace Engine
