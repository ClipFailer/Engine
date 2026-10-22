/**
 * @file Scene.cpp
 * @brief Implementation of Scene container, default level setup, and serialization.
 */

#include "Scene.hpp"
#include "Character.hpp"
#include "StaticMeshComponent.hpp"
#include "LightComponent.hpp"
#include "RigidBodyComponent.hpp"
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

    bool Scene::SaveToFile(const std::string& filepath) {
        try {
            json root;
            root["scene_name"] = "DefaultScene";
            root["actors"] = json::array();

            for (const auto& actor : m_Actors) {
                json a;
                a["uuid"] = static_cast<uint64>(actor->GetUUID());
                a["name"] = actor->GetName();
                a["class"] = std::string(actor->GetClassName());
                a["pos"] = {actor->Transform.Position.x, actor->Transform.Position.y, actor->Transform.Position.z};
                a["rot"] = {actor->Transform.Rotation.x, actor->Transform.Rotation.y, actor->Transform.Rotation.z};
                a["scale"] = {actor->Transform.Scale.x, actor->Transform.Scale.y, actor->Transform.Scale.z};

                a["components"] = json::array();
                for (const auto& comp : actor->GetComponents()) {
                    json c;
                    c["class"] = std::string(comp->GetClassName());
                    c["name"] = comp->GetName();

                    if (auto mesh = dynamic_cast<StaticMeshComponent*>(comp.get())) {
                        c["mesh_type"] = static_cast<uint8>(mesh->MeshType);
                        c["color"] = {mesh->BaseColor.r, mesh->BaseColor.g, mesh->BaseColor.b, mesh->BaseColor.a};
                        c["metallic"] = mesh->Metallic;
                        c["roughness"] = mesh->Roughness;
                    } else if (auto rb = dynamic_cast<RigidBodyComponent*>(comp.get())) {
                        c["mass"] = rb->Mass;
                        c["gravity"] = rb->bUseGravity;
                        c["static"] = rb->bIsStatic;
                        c["restitution"] = rb->Restitution;
                        c["friction"] = rb->Friction;
                    }
                    a["components"].push_back(c);
                }
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

                    Actor* actor = nullptr;
                    if (className == "Character") {
                        actor = SpawnActor<Character>(name);
                    } else if (className == "Directional Light" || name.find("Directional") != std::string::npos) {
                        actor = SpawnLightActor(name, LightType::Directional);
                    } else if (className == "Point Light" || name.find("Point") != std::string::npos) {
                        actor = SpawnLightActor(name, LightType::Point);
                    } else {
                        actor = SpawnActor(name);
                    }

                    if (a.contains("pos") && a["pos"].is_array() && a["pos"].size() >= 3) {
                        actor->Transform.Position = {a["pos"][0], a["pos"][1], a["pos"][2]};
                    }
                    if (a.contains("rot") && a["rot"].is_array() && a["rot"].size() >= 3) {
                        actor->Transform.Rotation = {a["rot"][0], a["rot"][1], a["rot"][2]};
                    }
                    if (a.contains("scale") && a["scale"].is_array() && a["scale"].size() >= 3) {
                        actor->Transform.Scale = {a["scale"][0], a["scale"][1], a["scale"][2]};
                    }

                    // Restore serialized component properties if present
                    if (a.contains("components") && a["components"].is_array()) {
                        for (const auto& c : a["components"]) {
                            std::string compClass = c.value("class", "");
                            if (compClass == "StaticMeshComponent") {
                                auto mesh = actor->GetComponent<StaticMeshComponent>();
                                if (!mesh) mesh = actor->AddComponent<StaticMeshComponent>();
                                if (c.contains("mesh_type")) {
                                    mesh->MeshType = static_cast<PrimitiveType>(c["mesh_type"].get<uint8>());
                                }
                                if (c.contains("color") && c["color"].size() >= 4) {
                                    mesh->BaseColor = {c["color"][0], c["color"][1], c["color"][2], c["color"][3]};
                                }
                                if (c.contains("metallic")) mesh->Metallic = c["metallic"];
                                if (c.contains("roughness")) mesh->Roughness = c["roughness"];
                            } else if (compClass == "RigidBodyComponent") {
                                auto rb = actor->GetComponent<RigidBodyComponent>();
                                if (!rb) rb = actor->AddComponent<RigidBodyComponent>();
                                if (c.contains("mass")) rb->Mass = c["mass"];
                                if (c.contains("gravity")) rb->bUseGravity = c["gravity"];
                                if (c.contains("static")) rb->bIsStatic = c["static"];
                                if (c.contains("restitution")) rb->Restitution = c["restitution"];
                                if (c.contains("friction")) rb->Friction = c["friction"];
                            }
                        }
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
