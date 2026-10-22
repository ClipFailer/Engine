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

    bool Scene::SaveToFile(const std::string& filepath) {
        try {
            json root;
            root["scene_name"] = "DefaultScene";
            root["actors"] = json::array();

            for (const auto& actor : m_Actors) {
                if (!actor) continue;
                json a;
                a["uuid"] = static_cast<uint64>(actor->GetUUID());
                a["name"] = actor->GetName();
                a["class"] = std::string(actor->GetClassName());
                a["tag"] = actor->Tag;
                a["visible"] = actor->bIsVisible;
                a["pos"] = {actor->Transform.Position.x, actor->Transform.Position.y, actor->Transform.Position.z};
                a["rot"] = {actor->Transform.Rotation.x, actor->Transform.Rotation.y, actor->Transform.Rotation.z};
                a["scale"] = {actor->Transform.Scale.x, actor->Transform.Scale.y, actor->Transform.Scale.z};

                if (auto chr = dynamic_cast<Character*>(actor.get())) {
                    a["health"] = chr->Health;
                    a["max_health"] = chr->MaxHealth;
                    a["walk_speed"] = chr->WalkSpeed;
                    a["jump_velocity"] = chr->JumpVelocity;
                }

                a["components"] = json::array();
                for (const auto& comp : actor->GetComponents()) {
                    if (!comp) continue;
                    json c;
                    c["class"] = std::string(comp->GetClassName());
                    c["name"] = comp->GetName();

                    if (auto mesh = dynamic_cast<StaticMeshComponent*>(comp.get())) {
                        c["mesh_type"] = static_cast<uint8>(mesh->MeshType);
                        c["color"] = {mesh->BaseColor.r, mesh->BaseColor.g, mesh->BaseColor.b, mesh->BaseColor.a};
                        c["metallic"] = mesh->Metallic;
                        c["roughness"] = mesh->Roughness;
                    } else if (auto light = dynamic_cast<LightComponent*>(comp.get())) {
                        c["light_type"] = static_cast<uint8>(light->Type);
                        c["color"] = {light->LightColor.r, light->LightColor.g, light->LightColor.b};
                        c["intensity"] = light->Intensity;
                        c["radius"] = light->Radius;
                    } else if (auto rb = dynamic_cast<RigidBodyComponent*>(comp.get())) {
                        c["mass"] = rb->Mass;
                        c["gravity"] = rb->bUseGravity;
                        c["static"] = rb->bIsStatic;
                        c["restitution"] = rb->Restitution;
                        c["friction"] = rb->Friction;
                        c["shape"] = static_cast<uint8>(rb->Shape);
                        c["dimensions"] = {rb->ColliderDimensions.x, rb->ColliderDimensions.y, rb->ColliderDimensions.z};
                    } else if (auto coll = dynamic_cast<CollisionComponent*>(comp.get())) {
                        c["shape"] = static_cast<uint8>(coll->Shape);
                        c["half_extents"] = {coll->BoxHalfExtents.x, coll->BoxHalfExtents.y, coll->BoxHalfExtents.z};
                        c["radius"] = coll->SphereRadius;
                        c["offset"] = {coll->Offset.x, coll->Offset.y, coll->Offset.z};
                        c["restitution"] = coll->Restitution;
                        c["friction"] = coll->Friction;
                        c["trigger"] = coll->bIsTrigger;
                    } else if (auto phys = dynamic_cast<PhysicsComponent*>(comp.get())) {
                        c["mass"] = phys->Mass;
                        c["gravity"] = phys->bUseGravity;
                        c["static"] = phys->bIsStatic;
                        c["linear_damping"] = phys->LinearDamping;
                        c["angular_damping"] = phys->AngularDamping;
                    } else if (auto move = dynamic_cast<MovementComponent*>(comp.get())) {
                        c["max_speed"] = move->MaxSpeed;
                        c["acceleration"] = move->Acceleration;
                        c["deceleration"] = move->Deceleration;
                    } else if (auto cust = dynamic_cast<CustomComponent*>(comp.get())) {
                        c["component_type"] = cust->ComponentType;
                        c["scalar"] = cust->CustomScalar;
                        c["enabled"] = cust->bIsEnabled;
                        c["tag"] = cust->CustomTag;
                    } else if (auto widget = dynamic_cast<WidgetComponent*>(comp.get())) {
                        c["html"] = widget->HTMLContent;
                        c["css"] = widget->CSSContent;
                        c["world_space"] = widget->bIsWorldSpace;
                        c["offset"] = {widget->Offset.x, widget->Offset.y, widget->Offset.z};
                        c["draw_size"] = {widget->DrawSize.x, widget->DrawSize.y};
                    } else if (auto audio = dynamic_cast<AudioComponent*>(comp.get())) {
                        c["sound_path"] = audio->SoundFilePath;
                        c["volume"] = audio->Volume;
                        c["pitch"] = audio->Pitch;
                        c["loop"] = audio->bLoop;
                        c["play_on_start"] = audio->bPlayOnStart;
                        c["is_3d"] = audio->bIs3D;
                        c["min_dist"] = audio->MinDistance;
                        c["max_dist"] = audio->MaxDistance;
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
                        bool hasSerializedComponents = a.contains("components") && a["components"].is_array() && !a["components"].empty();
                        actor = SpawnActor(name, /*bAttachDefaultComponents=*/!hasSerializedComponents);
                    }

                    if (a.contains("tag")) actor->Tag = a["tag"].get<std::string>();
                    if (a.contains("visible")) actor->bIsVisible = a["visible"].get<bool>();

                    if (a.contains("pos") && a["pos"].is_array() && a["pos"].size() >= 3) {
                        actor->Transform.Position = {a["pos"][0], a["pos"][1], a["pos"][2]};
                    }
                    if (a.contains("rot") && a["rot"].is_array() && a["rot"].size() >= 3) {
                        actor->Transform.Rotation = {a["rot"][0], a["rot"][1], a["rot"][2]};
                    }
                    if (a.contains("scale") && a["scale"].is_array() && a["scale"].size() >= 3) {
                        actor->Transform.Scale = {a["scale"][0], a["scale"][1], a["scale"][2]};
                    }

                    if (auto chr = dynamic_cast<Character*>(actor)) {
                        if (a.contains("health")) chr->Health = a["health"];
                        if (a.contains("max_health")) chr->MaxHealth = a["max_health"];
                        if (a.contains("walk_speed")) chr->WalkSpeed = a["walk_speed"];
                        if (a.contains("jump_velocity")) chr->JumpVelocity = a["jump_velocity"];
                    }

                    // Restore components
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
                            } else if (compClass == "LightComponent") {
                                auto light = actor->GetComponent<LightComponent>();
                                if (!light) light = actor->AddComponent<LightComponent>();
                                if (c.contains("light_type")) light->Type = static_cast<LightType>(c["light_type"].get<uint8>());
                                if (c.contains("color") && c["color"].size() >= 3) {
                                    light->LightColor = {c["color"][0], c["color"][1], c["color"][2]};
                                }
                                if (c.contains("intensity")) light->Intensity = c["intensity"];
                                if (c.contains("radius")) light->Radius = c["radius"];
                            } else if (compClass == "RigidBodyComponent") {
                                auto rb = actor->GetComponent<RigidBodyComponent>();
                                if (!rb) rb = actor->AddComponent<RigidBodyComponent>();
                                if (c.contains("mass")) rb->Mass = c["mass"];
                                if (c.contains("gravity")) rb->bUseGravity = c["gravity"];
                                if (c.contains("static")) rb->bIsStatic = c["static"];
                                if (c.contains("restitution")) rb->Restitution = c["restitution"];
                                if (c.contains("friction")) rb->Friction = c["friction"];
                                if (c.contains("shape")) rb->Shape = static_cast<ColliderType>(c["shape"].get<uint8>());
                                if (c.contains("dimensions") && c["dimensions"].size() >= 3) {
                                    rb->ColliderDimensions = {c["dimensions"][0], c["dimensions"][1], c["dimensions"][2]};
                                }
                            } else if (compClass == "CollisionComponent") {
                                auto coll = actor->GetComponent<CollisionComponent>();
                                if (!coll) coll = actor->AddComponent<CollisionComponent>();
                                if (c.contains("shape")) coll->Shape = static_cast<CollisionShape>(c["shape"].get<uint8>());
                                if (c.contains("half_extents") && c["half_extents"].size() >= 3) {
                                    coll->BoxHalfExtents = {c["half_extents"][0], c["half_extents"][1], c["half_extents"][2]};
                                }
                                if (c.contains("radius")) coll->SphereRadius = c["radius"];
                                if (c.contains("offset") && c["offset"].size() >= 3) {
                                    coll->Offset = {c["offset"][0], c["offset"][1], c["offset"][2]};
                                }
                                if (c.contains("restitution")) coll->Restitution = c["restitution"];
                                if (c.contains("friction")) coll->Friction = c["friction"];
                                if (c.contains("trigger")) coll->bIsTrigger = c["trigger"];
                            } else if (compClass == "PhysicsComponent") {
                                auto phys = actor->GetComponent<PhysicsComponent>();
                                if (!phys) phys = actor->AddComponent<PhysicsComponent>();
                                if (c.contains("mass")) phys->Mass = c["mass"];
                                if (c.contains("gravity")) phys->bUseGravity = c["gravity"];
                                if (c.contains("static")) phys->bIsStatic = c["static"];
                                if (c.contains("linear_damping")) phys->LinearDamping = c["linear_damping"];
                                if (c.contains("angular_damping")) phys->AngularDamping = c["angular_damping"];
                            } else if (compClass == "MovementComponent") {
                                auto move = actor->GetComponent<MovementComponent>();
                                if (!move) move = actor->AddComponent<MovementComponent>();
                                if (c.contains("max_speed")) move->MaxSpeed = c["max_speed"];
                                if (c.contains("acceleration")) move->Acceleration = c["acceleration"];
                                if (c.contains("deceleration")) move->Deceleration = c["deceleration"];
                            } else if (compClass == "CustomComponent") {
                                std::string typeName = c.value("component_type", "CustomComponent");
                                auto cust = actor->AddComponent<CustomComponent>(typeName);
                                if (c.contains("scalar")) cust->CustomScalar = c["scalar"];
                                if (c.contains("enabled")) cust->bIsEnabled = c["enabled"];
                                if (c.contains("tag")) cust->CustomTag = c["tag"].get<std::string>();
                            } else if (compClass == "WidgetComponent") {
                                std::string html = c.value("html", "");
                                std::string css = c.value("css", "");
                                auto widget = actor->AddComponent<WidgetComponent>(html, css);
                                if (c.contains("world_space")) widget->bIsWorldSpace = c["world_space"];
                                if (c.contains("offset") && c["offset"].size() >= 3) {
                                    widget->Offset = {c["offset"][0], c["offset"][1], c["offset"][2]};
                                }
                                if (c.contains("draw_size") && c["draw_size"].size() >= 2) {
                                    widget->DrawSize = {c["draw_size"][0], c["draw_size"][1]};
                                }
                            } else if (compClass == "AudioComponent") {
                                auto audio = actor->AddComponent<AudioComponent>();
                                if (c.contains("sound_path")) audio->SoundFilePath = c["sound_path"].get<std::string>();
                                if (c.contains("volume")) audio->Volume = c["volume"];
                                if (c.contains("pitch")) audio->Pitch = c["pitch"];
                                if (c.contains("loop")) audio->bLoop = c["loop"];
                                if (c.contains("play_on_start")) audio->bPlayOnStart = c["play_on_start"];
                                if (c.contains("is_3d")) audio->bIs3D = c["is_3d"];
                                if (c.contains("min_dist")) audio->MinDistance = c["min_dist"];
                                if (c.contains("max_dist")) audio->MaxDistance = c["max_dist"];
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
