/**
 * @file OutlinerPanel.cpp
 * @brief Implementation of OutlinerPanel.
 */

#include "OutlinerPanel.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "scene/ShaderActor.hpp"
#include "core/Log.hpp"
#include <imgui.h>
#include <filesystem>
#include <string>

namespace Engine {

    void OutlinerPanel::Render(Scene& scene) {
        ImGui::Begin("Outliner");

        // Validate that m_SelectedActor still exists in the active scene
        if (m_SelectedActor) {
            bool bFound = false;
            for (const auto& a : scene.GetActors()) {
                if (a.get() == m_SelectedActor) {
                    bFound = true;
                    break;
                }
            }
            if (!bFound) {
                m_SelectedActor = nullptr;
            }
        }

        // Search Bar
        ImGui::InputTextWithHint("##Filter", "Search Actors...", m_SearchFilter, sizeof(m_SearchFilter));
        ImGui::Separator();

        // Actor List
        ImGui::BeginChild("ActorList", ImVec2(0, 0), false);

        std::string filterStr(m_SearchFilter);

        for (const auto& actor : scene.GetActors()) {
            if (!filterStr.empty() && actor->GetName().find(filterStr) == std::string::npos) {
                continue;
            }

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (m_SelectedActor == actor.get()) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint64)actor->GetUUID(), flags, "%s", actor->GetName().c_str());

            if (ImGui::IsItemClicked()) {
                m_SelectedActor = actor.get();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (OnActorDoubleClicked) {
                    OnActorDoubleClicked(actor.get());
                }
            }

            // Actor Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Open Actor Detail Window")) {
                    if (OnActorDoubleClicked) OnActorDoubleClicked(actor.get());
                }
                if (ImGui::MenuItem("Delete Actor")) {
                    if (m_SelectedActor == actor.get()) m_SelectedActor = nullptr;
                    scene.DestroyActor(actor.get());
                    ImGui::EndPopup();
                    if (opened) ImGui::TreePop();
                    break;
                }
                ImGui::EndPopup();
            }

            if (opened) {
                for (const auto& comp : actor->GetComponents()) {
                    ImGui::BulletText("%s (%s)", comp->GetName().c_str(), comp->GetClassName().data());
                }
                ImGui::TreePop();
            }
        }

        // Delete key handling in Outliner
        if (m_SelectedActor && ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            scene.DestroyActor(m_SelectedActor);
            m_SelectedActor = nullptr;
        }

        // Window background right-click context menu (Add Actor)
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::BeginMenu("Add Actor")) {
                if (ImGui::MenuItem("Cube Actor")) {
                    auto cube = scene.SpawnActor("Cube Actor");
                    cube->Transform.Position = glm::vec3(0.0f, 2.0f, 0.0f);
                    m_SelectedActor = cube;
                }
                if (ImGui::MenuItem("Sphere Actor")) {
                    auto sphere = scene.SpawnActor("Sphere Actor");
                    sphere->Transform.Position = glm::vec3(0.0f, 2.0f, 0.0f);
                    if (auto mesh = sphere->GetComponent<StaticMeshComponent>()) {
                        mesh->MeshType = PrimitiveType::Sphere;
                    }
                    m_SelectedActor = sphere;
                }
                if (ImGui::MenuItem("Directional Light")) {
                    auto light = scene.SpawnLightActor("Directional Light", LightType::Directional);
                    m_SelectedActor = light;
                }
                if (ImGui::MenuItem("Point Light")) {
                    auto light = scene.SpawnLightActor("Point Light", LightType::Point);
                    light->Transform.Position = glm::vec3(0.0f, 3.0f, 0.0f);
                    if (auto pl = light->GetComponent<LightComponent>()) {
                        pl->Radius = 10.0f;
                        pl->Intensity = 2.0f;
                    }
                    m_SelectedActor = light;
                }
                if (ImGui::MenuItem("Shader Volume (Trigger / Post-Process)")) {
                    auto shaderActor = scene.SpawnActor<ShaderActor>("Shader Volume");
                    shaderActor->Transform.Position = glm::vec3(0.0f, 1.0f, 0.0f);
                    m_SelectedActor = shaderActor;
                }
                if (ImGui::MenuItem("Empty Actor")) {
                    m_SelectedActor = scene.SpawnActor("Empty Actor");
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        // Drag and drop target to spawn actor from Content Browser into Scene via Outliner
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ACTOR_ASSET")) {
                std::string payloadData = (const char*)payload->Data;
                std::filesystem::path assetPath(payloadData);
                glm::vec3 spawnPos{0.0f, 1.5f, 0.0f};

                Actor* spawned = nullptr;
                if (assetPath.extension() == ".actor" && std::filesystem::exists(assetPath)) {
                    spawned = scene.SpawnActorFromAsset(assetPath, spawnPos);
                } else {
                    std::filesystem::path candidate = std::filesystem::current_path() / "Content" / (assetPath.stem().string() + ".actor");
                    if (std::filesystem::exists(candidate)) {
                        spawned = scene.SpawnActorFromAsset(candidate, spawnPos);
                    } else {
                        spawned = scene.SpawnActor(assetPath.stem().string());
                        spawned->Transform.Position = spawnPos;
                    }
                }
                if (spawned) {
                    m_SelectedActor = spawned;
                }
                LOG_INFO("Spawned Actor '{}' into Scene via Outliner Drag-and-Drop", spawned ? spawned->GetName() : payloadData);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::EndChild();
        ImGui::End();
    }

} // namespace Engine
