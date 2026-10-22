/**
 * @file Actor.cpp
 * @brief Implementation and reflection metadata for Actor.
 */

#include "Actor.hpp"
#include "Character.hpp"
#include "PositionComponent.hpp"
#include "StaticMeshComponent.hpp"
#include "PhysicsComponent.hpp"
#include "CollisionComponent.hpp"
#include "LightComponent.hpp"
#include "RigidBodyComponent.hpp"
#include "MovementComponent.hpp"
#include "CustomComponent.hpp"
#include "WidgetComponent.hpp"
#include "AudioComponent.hpp"
#include "ShaderComponent.hpp"
#include "animation/SkeletalMeshComponent.hpp"
#include "core/Log.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace Engine {

    using json = nlohmann::json;

    Actor::Actor() : Actor("Actor", true) {}

    Actor::Actor(std::string name, bool bAttachDefaultComponents) : Object(std::move(name)) {
        AddComponent<PositionComponent>();
        if (bAttachDefaultComponents) {
            AddComponent<StaticMeshComponent>(PrimitiveType::Cube);
            AddComponent<PhysicsComponent>();
            AddComponent<CollisionComponent>();
        }
    }

    void Actor::BeginPlay() {
        for (auto& comp : m_Components) {
            if (comp->bIsActive) {
                comp->BeginPlay();
            }
        }
    }

    void Actor::Tick(float deltaTime) {
        for (auto& comp : m_Components) {
            if (comp->bIsActive) {
                comp->Tick(deltaTime);
            }
        }
    }

    void Actor::EndPlay() {
        for (auto& comp : m_Components) {
            if (comp->bIsActive) {
                comp->EndPlay();
            }
        }
    }

    void Actor::RemoveComponent(Component* comp) {
        auto it = std::remove_if(m_Components.begin(), m_Components.end(),
            [comp](const std::unique_ptr<Component>& ptr) {
                return ptr.get() == comp;
            });
        m_Components.erase(it, m_Components.end());
    }

    nlohmann::json Actor::Serialize(bool bIncludePosition) const {
        json a;
        a["name"] = GetName();
        a["class"] = std::string(GetClassName());
        a["tag"] = Tag;
        a["visible"] = bIsVisible;
        a["scale"] = {Transform.Scale.x, Transform.Scale.y, Transform.Scale.z};
        a["rot"] = {Transform.Rotation.x, Transform.Rotation.y, Transform.Rotation.z};
        if (bIncludePosition) {
            a["pos"] = {Transform.Position.x, Transform.Position.y, Transform.Position.z};
        }
        if (!AssetPath.empty()) {
            a["asset_path"] = AssetPath;
        }

        if (auto chr = dynamic_cast<const Character*>(this)) {
            a["health"] = chr->Health;
            a["max_health"] = chr->MaxHealth;
            a["walk_speed"] = chr->WalkSpeed;
            a["jump_velocity"] = chr->JumpVelocity;
        }

        a["components"] = json::array();
        for (const auto& comp : m_Components) {
            if (!comp) continue;
            json c;
            c["class"] = std::string(comp->GetClassName());
            c["name"] = comp->GetName();

            if (auto mesh = dynamic_cast<StaticMeshComponent*>(comp.get())) {
                c["mesh_type"] = static_cast<uint8>(mesh->MeshType);
                c["color"] = {mesh->BaseColor.r, mesh->BaseColor.g, mesh->BaseColor.b, mesh->BaseColor.a};
                c["metallic"] = mesh->Metallic;
                c["roughness"] = mesh->Roughness;
                c["cast_shadows"] = mesh->bCastShadows;
                c["model_path"] = mesh->ModelPath;
                c["texture_path"] = mesh->TexturePath;
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
                c["linear_velocity"] = {phys->LinearVelocity.x, phys->LinearVelocity.y, phys->LinearVelocity.z};
                c["angular_velocity"] = {phys->AngularVelocity.x, phys->AngularVelocity.y, phys->AngularVelocity.z};
            } else if (auto rb = dynamic_cast<RigidBodyComponent*>(comp.get())) {
                c["mass"] = rb->Mass;
                c["gravity"] = rb->bUseGravity;
                c["static"] = rb->bIsStatic;
                c["restitution"] = rb->Restitution;
                c["friction"] = rb->Friction;
                c["shape"] = static_cast<uint8>(rb->Shape);
                c["dimensions"] = {rb->ColliderDimensions.x, rb->ColliderDimensions.y, rb->ColliderDimensions.z};
            } else if (auto light = dynamic_cast<LightComponent*>(comp.get())) {
                c["light_type"] = static_cast<uint8>(light->Type);
                c["color"] = {light->LightColor.r, light->LightColor.g, light->LightColor.b};
                c["intensity"] = light->Intensity;
                c["radius"] = light->Radius;
                c["inner_cone"] = light->InnerConeAngle;
                c["outer_cone"] = light->OuterConeAngle;
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
            } else if (auto skel = dynamic_cast<SkeletalMeshComponent*>(comp.get())) {
                c["playback_speed"] = skel->PlaybackSpeed;
                c["loop"] = skel->bLoop;
                c["is_playing"] = skel->bIsPlaying;
            } else if (auto sc = dynamic_cast<ShaderComponent*>(comp.get())) {
                c["shader_name"] = sc->ShaderName;
                c["shader_code"] = sc->ShaderCode;
                c["effect_type"] = static_cast<uint8>(sc->EffectType);
                c["effect_intensity"] = sc->EffectIntensity;
                c["active_everywhere"] = sc->bActiveEverywhere;
                c["is_sphere_zone"] = sc->bIsSphereZone;
                c["trigger_half_extents"] = {sc->TriggerHalfExtents.x, sc->TriggerHalfExtents.y, sc->TriggerHalfExtents.z};
                c["trigger_radius"] = sc->TriggerRadius;
                c["trigger_offset"] = {sc->TriggerOffset.x, sc->TriggerOffset.y, sc->TriggerOffset.z};
            }
            a["components"].push_back(c);
        }
        return a;
    }

    bool Actor::Deserialize(const nlohmann::json& j, bool bRestorePosition) {
        try {
            if (j.contains("name")) SetName(j["name"].get<std::string>());
            if (j.contains("tag")) Tag = j["tag"].get<std::string>();
            if (j.contains("visible")) bIsVisible = j["visible"].get<bool>();
            if (j.contains("asset_path")) AssetPath = j["asset_path"].get<std::string>();

            if (j.contains("scale") && j["scale"].is_array() && j["scale"].size() >= 3) {
                Transform.Scale = {j["scale"][0], j["scale"][1], j["scale"][2]};
            }
            if (j.contains("rot") && j["rot"].is_array() && j["rot"].size() >= 3) {
                Transform.Rotation = {j["rot"][0], j["rot"][1], j["rot"][2]};
            }
            if (bRestorePosition && j.contains("pos") && j["pos"].is_array() && j["pos"].size() >= 3) {
                Transform.Position = {j["pos"][0], j["pos"][1], j["pos"][2]};
            }

            if (auto posComp = GetComponent<PositionComponent>()) {
                posComp->Transform = Transform;
            }

            if (auto chr = dynamic_cast<Character*>(this)) {
                if (j.contains("health")) chr->Health = j["health"];
                if (j.contains("max_health")) chr->MaxHealth = j["max_health"];
                if (j.contains("walk_speed")) chr->WalkSpeed = j["walk_speed"];
                if (j.contains("jump_velocity")) chr->JumpVelocity = j["jump_velocity"];
            }

            if (j.contains("components") && j["components"].is_array()) {
                for (const auto& c : j["components"]) {
                    std::string compClass = c.value("class", "");
                    if (compClass == "StaticMeshComponent") {
                        auto mesh = GetComponent<StaticMeshComponent>();
                        if (!mesh) mesh = AddComponent<StaticMeshComponent>();
                        if (c.contains("mesh_type")) {
                            mesh->MeshType = static_cast<PrimitiveType>(c["mesh_type"].get<uint8>());
                        }
                        if (c.contains("color") && c["color"].size() >= 4) {
                            mesh->BaseColor = {c["color"][0], c["color"][1], c["color"][2], c["color"][3]};
                        }
                        if (c.contains("metallic")) mesh->Metallic = c["metallic"];
                        if (c.contains("roughness")) mesh->Roughness = c["roughness"];
                        if (c.contains("cast_shadows")) mesh->bCastShadows = c["cast_shadows"];
                        if (c.contains("model_path") && !c["model_path"].get<std::string>().empty()) {
                            mesh->LoadModel(c["model_path"].get<std::string>());
                        }
                        if (c.contains("texture_path")) {
                            mesh->TexturePath = c["texture_path"].get<std::string>();
                        }
                    } else if (compClass == "CollisionComponent") {
                        auto coll = GetComponent<CollisionComponent>();
                        if (!coll) coll = AddComponent<CollisionComponent>();
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
                        auto phys = GetComponent<PhysicsComponent>();
                        if (!phys) phys = AddComponent<PhysicsComponent>();
                        if (c.contains("mass")) phys->Mass = c["mass"];
                        if (c.contains("gravity")) phys->bUseGravity = c["gravity"];
                        if (c.contains("static")) phys->bIsStatic = c["static"];
                        if (c.contains("linear_damping")) phys->LinearDamping = c["linear_damping"];
                        if (c.contains("angular_damping")) phys->AngularDamping = c["angular_damping"];
                        if (c.contains("linear_velocity") && c["linear_velocity"].size() >= 3) {
                            phys->LinearVelocity = {c["linear_velocity"][0], c["linear_velocity"][1], c["linear_velocity"][2]};
                        }
                        if (c.contains("angular_velocity") && c["angular_velocity"].size() >= 3) {
                            phys->AngularVelocity = {c["angular_velocity"][0], c["angular_velocity"][1], c["angular_velocity"][2]};
                        }
                    } else if (compClass == "RigidBodyComponent") {
                        auto rb = GetComponent<RigidBodyComponent>();
                        if (!rb) rb = AddComponent<RigidBodyComponent>();
                        if (c.contains("mass")) rb->Mass = c["mass"];
                        if (c.contains("gravity")) rb->bUseGravity = c["gravity"];
                        if (c.contains("static")) rb->bIsStatic = c["static"];
                        if (c.contains("restitution")) rb->Restitution = c["restitution"];
                        if (c.contains("friction")) rb->Friction = c["friction"];
                        if (c.contains("shape")) rb->Shape = static_cast<ColliderType>(c["shape"].get<uint8>());
                        if (c.contains("dimensions") && c["dimensions"].size() >= 3) {
                            rb->ColliderDimensions = {c["dimensions"][0], c["dimensions"][1], c["dimensions"][2]};
                        }
                    } else if (compClass == "LightComponent") {
                        auto light = GetComponent<LightComponent>();
                        if (!light) light = AddComponent<LightComponent>();
                        if (c.contains("light_type")) light->Type = static_cast<LightType>(c["light_type"].get<uint8>());
                        if (c.contains("color") && c["color"].size() >= 3) {
                            light->LightColor = {c["color"][0], c["color"][1], c["color"][2]};
                        }
                        if (c.contains("intensity")) light->Intensity = c["intensity"];
                        if (c.contains("radius")) light->Radius = c["radius"];
                        if (c.contains("inner_cone")) light->InnerConeAngle = c["inner_cone"];
                        if (c.contains("outer_cone")) light->OuterConeAngle = c["outer_cone"];
                    } else if (compClass == "MovementComponent") {
                        auto move = GetComponent<MovementComponent>();
                        if (!move) move = AddComponent<MovementComponent>();
                        if (c.contains("max_speed")) move->MaxSpeed = c["max_speed"];
                        if (c.contains("acceleration")) move->Acceleration = c["acceleration"];
                        if (c.contains("deceleration")) move->Deceleration = c["deceleration"];
                    } else if (compClass == "CustomComponent") {
                        std::string ctype = c.value("component_type", "CustomComponent");
                        auto cust = GetComponent<CustomComponent>();
                        if (!cust) cust = AddComponent<CustomComponent>(ctype);
                        if (c.contains("scalar")) cust->CustomScalar = c["scalar"];
                        if (c.contains("enabled")) cust->bIsEnabled = c["enabled"];
                        if (c.contains("tag")) cust->CustomTag = c["tag"].get<std::string>();
                    } else if (compClass == "WidgetComponent") {
                        auto widget = GetComponent<WidgetComponent>();
                        if (!widget) widget = AddComponent<WidgetComponent>();
                        if (c.contains("html")) widget->HTMLContent = c["html"].get<std::string>();
                        if (c.contains("css")) widget->CSSContent = c["css"].get<std::string>();
                        if (c.contains("world_space")) widget->bIsWorldSpace = c["world_space"].get<bool>();
                        if (c.contains("offset") && c["offset"].size() >= 3) {
                            widget->Offset = {c["offset"][0], c["offset"][1], c["offset"][2]};
                        }
                        if (c.contains("draw_size") && c["draw_size"].size() >= 2) {
                            widget->DrawSize = {c["draw_size"][0], c["draw_size"][1]};
                        }
                    } else if (compClass == "AudioComponent") {
                        auto audio = GetComponent<AudioComponent>();
                        if (!audio) audio = AddComponent<AudioComponent>();
                        if (c.contains("sound_path")) audio->SoundFilePath = c["sound_path"].get<std::string>();
                        if (c.contains("volume")) audio->Volume = c["volume"];
                        if (c.contains("pitch")) audio->Pitch = c["pitch"];
                        if (c.contains("loop")) audio->bLoop = c["loop"];
                        if (c.contains("play_on_start")) audio->bPlayOnStart = c["play_on_start"];
                        if (c.contains("is_3d")) audio->bIs3D = c["is_3d"];
                        if (c.contains("min_dist")) audio->MinDistance = c["min_dist"];
                        if (c.contains("max_dist")) audio->MaxDistance = c["max_dist"];
                    } else if (compClass == "SkeletalMeshComponent") {
                        auto skel = GetComponent<SkeletalMeshComponent>();
                        if (!skel) skel = AddComponent<SkeletalMeshComponent>();
                        if (c.contains("playback_speed")) skel->PlaybackSpeed = c["playback_speed"];
                        if (c.contains("loop")) skel->bLoop = c["loop"];
                        if (c.contains("is_playing")) skel->bIsPlaying = c["is_playing"];
                    } else if (compClass == "ShaderComponent") {
                        auto sc = GetComponent<ShaderComponent>();
                        if (!sc) sc = AddComponent<ShaderComponent>();
                        if (c.contains("shader_name")) sc->ShaderName = c["shader_name"].get<std::string>();
                        if (c.contains("shader_code")) sc->ShaderCode = c["shader_code"].get<std::string>();
                        if (c.contains("effect_type")) sc->EffectType = static_cast<ShaderEffectType>(c["effect_type"].get<uint8>());
                        if (c.contains("effect_intensity")) sc->EffectIntensity = c["effect_intensity"].get<float>();
                        if (c.contains("active_everywhere")) sc->bActiveEverywhere = c["active_everywhere"].get<bool>();
                        if (c.contains("is_sphere_zone")) sc->bIsSphereZone = c["is_sphere_zone"].get<bool>();
                        if (c.contains("trigger_half_extents") && c["trigger_half_extents"].size() >= 3) {
                            sc->TriggerHalfExtents = {c["trigger_half_extents"][0], c["trigger_half_extents"][1], c["trigger_half_extents"][2]};
                        }
                        if (c.contains("trigger_radius")) sc->TriggerRadius = c["trigger_radius"].get<float>();
                        if (c.contains("trigger_offset") && c["trigger_offset"].size() >= 3) {
                            sc->TriggerOffset = {c["trigger_offset"][0], c["trigger_offset"][1], c["trigger_offset"][2]};
                        }
                    }
                }
            }
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Exception in Actor::Deserialize: {}", e.what());
            return false;
        }
    }

    bool Actor::SaveToAssetFile(const std::string& filepath) const {
        try {
            std::filesystem::path p(filepath);
            if (p.has_parent_path()) {
                std::filesystem::create_directories(p.parent_path());
            }
            json j = Serialize(/*bIncludePosition=*/false);
            std::ofstream out(filepath);
            if (!out.is_open()) return false;
            out << j.dump(4);
            out.close();
            const_cast<Actor*>(this)->AssetPath = filepath;
            LOG_INFO("Saved Actor Asset to {}", filepath);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save Actor Asset to {}: {}", filepath, e.what());
            return false;
        }
    }

    bool Actor::LoadFromAssetFile(const std::string& filepath) {
        try {
            std::ifstream in(filepath);
            if (!in.is_open()) return false;
            json j;
            in >> j;
            in.close();
            bool ok = Deserialize(j, /*bRestorePosition=*/false);
            if (ok) {
                AssetPath = filepath;
                LOG_INFO("Loaded Actor Asset from {}", filepath);
            }
            return ok;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load Actor Asset from {}: {}", filepath, e.what());
            return false;
        }
    }

    ENGINE_CLASS_BEGIN(Actor, Object)
        ENGINE_PROPERTY(Actor, Transform, Transform, "Transform", "Spatial transform (Location, Rotation, Scale)")
        ENGINE_PROPERTY(Actor, Tag, String, "General", "Actor category tag")
        ENGINE_PROPERTY(Actor, bIsVisible, Bool, "Rendering", "Visibility flag in scene")
    ENGINE_CLASS_END()

} // namespace Engine
