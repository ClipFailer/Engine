/**
 * @file Scene.cpp
 * @brief Implementation of Scene container, default level setup, and serialization.
 */

#include "Scene.hpp"
#include "Character.hpp"
#include "StaticMeshComponent.hpp"
#include "LightComponent.hpp"
#include "RigidBodyComponent.hpp"
#include "CollisionComponent.hpp"
#include "PhysicsComponent.hpp"
#include "MovementComponent.hpp"
#include "CustomComponent.hpp"
#include "WidgetComponent.hpp"
#include "AudioComponent.hpp"
#include "core/Log.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

namespace Engine {

    using json = nlohmann::json;

    Scene::Scene() {
        SetupDefaultScene();
    }

    Scene::~Scene() {
        if (m_State != SimulationState::Stopped) {
            StopSimulation();
        }
    }

    Actor* Scene::SpawnLightActor(const std::string& name, LightType type) {
        auto actor = std::make_unique<Actor>(name, /*bAttachDefaultComponents=*/false);
        actor->AddComponent<LightComponent>(type);
        Actor* ptr = actor.get();
        m_Actors.push_back(std::move(actor));
        return ptr;
    }

    void Scene::DestroyActor(Actor* actor) {
        if (!actor) return;
        auto it = std::remove_if(m_Actors.begin(), m_Actors.end(),
            [actor](const std::unique_ptr<Actor>& ptr) {
                return ptr.get() == actor;
            });
        m_Actors.erase(it, m_Actors.end());
    }

    Actor* Scene::FindActorByUUID(UUID id) const {
        for (const auto& actor : m_Actors) {
            if (actor->GetUUID() == id) {
                return actor.get();
            }
        }
        return nullptr;
    }

    void Scene::StartSimulation() {
        if (m_State != SimulationState::Stopped) return;

        m_InitialSnapshots.clear();
        for (const auto& actor : m_Actors) {
            m_InitialSnapshots.push_back({actor->GetUUID(), actor->Transform});
            actor->BeginPlay();
        }

        m_State = SimulationState::Playing;
        LOG_INFO("Simulation started (Play in Editor)");
    }

    void Scene::Tick(float deltaTime) {
        if (m_State != SimulationState::Playing) return;

        for (auto& actor : m_Actors) {
            actor->Tick(deltaTime);
        }
    }

    void Scene::StopSimulation() {
        if (m_State == SimulationState::Stopped) return;

        for (auto& actor : m_Actors) {
            actor->EndPlay();
        }

        // Restore initial transforms
        for (const auto& snap : m_InitialSnapshots) {
            if (Actor* actor = FindActorByUUID(snap.Id)) {
                actor->Transform = snap.Transform;
                if (auto rb = actor->GetComponent<RigidBodyComponent>()) {
                    rb->LinearVelocity = glm::vec3(0.0f);
                    rb->AngularVelocity = glm::vec3(0.0f);
                }
            }
        }

        m_State = SimulationState::Stopped;
        LOG_INFO("Simulation stopped (Reset to Editor state)");
    }

    void Scene::SetupDefaultScene() {
        if (OnActorsCleared) OnActorsCleared();
        m_Actors.clear();

        // 1. Directional Sun Light (lightweight entity: no mesh, no physics, no collision)
        auto sun = SpawnLightActor("Directional Light", LightType::Directional);
        sun->Transform.Position = glm::vec3(5.0f, 10.0f, 5.0f);
        sun->Transform.Rotation = glm::vec3(-45.0f, 35.0f, 0.0f);

        // 2. Ground Floor Plane
        auto ground = SpawnActor("Ground Plane");
        ground->Transform.Position = glm::vec3(0.0f, 0.0f, 0.0f);
        auto groundMesh = ground->AddComponent<StaticMeshComponent>(PrimitiveType::Plane);
        groundMesh->BaseColor = glm::vec4(0.25f, 0.28f, 0.32f, 1.0f);
        auto groundRb = ground->AddComponent<RigidBodyComponent>();
        groundRb->bIsStatic = true;
        groundRb->bUseGravity = false;
        groundRb->Shape = ColliderType::Box;
        groundRb->ColliderDimensions = glm::vec3(10.0f, 0.1f, 10.0f);

        // 3. Dynamic Physics Cube
        auto cube = SpawnActor("Cube Actor");
        cube->Transform.Position = glm::vec3(-1.5f, 4.0f, 0.0f);
        auto cubeMesh = cube->AddComponent<StaticMeshComponent>(PrimitiveType::Cube);
        cubeMesh->BaseColor = glm::vec4(0.85f, 0.35f, 0.25f, 1.0f);
        auto cubeRb = cube->AddComponent<RigidBodyComponent>();
        cubeRb->Mass = 2.0f;
        cubeRb->Shape = ColliderType::Box;
        cubeRb->ColliderDimensions = glm::vec3(0.5f);

        // 4. Dynamic Physics Sphere (Bouncing)
        auto sphere = SpawnActor("Bouncing Sphere");
        sphere->Transform.Position = glm::vec3(1.5f, 6.0f, 0.0f);
        auto sphereMesh = sphere->AddComponent<StaticMeshComponent>(PrimitiveType::Sphere);
        sphereMesh->BaseColor = glm::vec4(0.2f, 0.6f, 0.9f, 1.0f);
        sphereMesh->Metallic = 0.8f;
        sphereMesh->Roughness = 0.2f;
        auto sphereRb = sphere->AddComponent<RigidBodyComponent>();
        sphereRb->Mass = 1.0f;
        sphereRb->Restitution = 0.85f; // High bounciness!
        sphereRb->Shape = ColliderType::Sphere;
        sphereRb->ColliderDimensions = glm::vec3(0.5f);

        LOG_INFO("Default 3D scene loaded with primitives, physics, and lighting");
    }

    Actor* Scene::SpawnActorFromAsset(const std::filesystem::path& assetPath, const glm::vec3& spawnPosition) {
        std::string stem = assetPath.stem().string();
        std::string className = "Actor";
        try {
            std::ifstream in(assetPath);
            if (in.is_open()) {
                json j;
                in >> j;
                if (j.contains("class")) className = j["class"].get<std::string>();
            }
        } catch (...) {}

        Actor* actor = nullptr;
        if (className == "Character") {
            actor = SpawnActor<Character>(stem);
        } else {
            actor = SpawnActor(stem, /*bAttachDefaultComponents=*/false);
        }

        if (actor) {
            actor->LoadFromAssetFile(assetPath.string());
            actor->Transform.Position = spawnPosition;
            actor->AssetPath = assetPath.string();
            LOG_INFO("Spawned Actor '{}' from asset '{}' at ({:.2f}, {:.2f}, {:.2f})",
                     actor->GetName(), assetPath.filename().string(), spawnPosition.x, spawnPosition.y, spawnPosition.z);
        }
        return actor;
    }

    bool Scene::SaveToFile(const std::string& filepath) {
        try {
            json root;
            root["scene_name"] = "DefaultScene";
            root["actors"] = json::array();

            for (const auto& actor : m_Actors) {
                if (!actor) continue;
                json a = actor->Serialize(/*bIncludePosition=*/true);
                a["uuid"] = static_cast<uint64>(actor->GetUUID());
                root["actors"].push_back(a);
            }

            std::ofstream file(filepath);
            if (!file.is_open()) return false;
            file << root.dump(4);
            LOG_INFO("Saved scene successfully to {}", filepath);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save scene {}: {}", filepath, e.what());
            return false;
        }
    }

    bool Scene::LoadFromFile(const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) return false;
            json root;
            file >> root;

            if (OnActorsCleared) OnActorsCleared();
            m_Actors.clear();

            if (root.contains("actors") && root["actors"].is_array()) {
                for (const auto& a : root["actors"]) {
                    std::string name = a.value("name", "Actor");
                    std::string className = a.value("class", "Actor");

                    bool isLight = (className == "Directional Light" || className == "Point Light" ||
                                   name.find("Directional") != std::string::npos || name.find("Point Light") != std::string::npos);

                    Actor* actor = nullptr;
                    if (className == "Character") {
                        actor = SpawnActor<Character>(name);
                    } else if (isLight) {
                        LightType lt = (className == "Point Light" || name.find("Point") != std::string::npos) ?
                                       LightType::Point : LightType::Directional;
                        actor = SpawnLightActor(name, lt);
                    } else {
                        actor = SpawnActor(name, /*bAttachDefaultComponents=*/false);
                    }

                    if (actor) {
                        actor->Deserialize(a, /*bRestorePosition=*/true);
                    }
                }
            }
            LOG_INFO("Loaded scene from {}", filepath);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load scene {}: {}", filepath, e.what());
            return false;
        }
    }

} // namespace Engine
