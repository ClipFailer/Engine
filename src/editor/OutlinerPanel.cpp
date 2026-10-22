/**
 * @file OutlinerPanel.cpp
 * @brief Implementation of OutlinerPanel.
 */

#include "OutlinerPanel.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include <imgui.h>
#include <string>

namespace Engine {

    void OutlinerPanel::Render(Scene& scene) {
        ImGui::Begin("Outliner");

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

            // Actor Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
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

        // Window background right-click context menu (Add Actor)
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::BeginMenu("Add Actor")) {
                if (ImGui::MenuItem("Cube Actor")) {
                    auto cube = scene.SpawnActor("Cube Actor");
                    cube->Transform.Position = glm::vec3(0.0f, 2.0f, 0.0f);
                    cube->AddComponent<StaticMeshComponent>(PrimitiveType::Cube);
                    cube->AddComponent<RigidBodyComponent>();
                    m_SelectedActor = cube;
                }
                if (ImGui::MenuItem("Sphere Actor")) {
                    auto sphere = scene.SpawnActor("Sphere Actor");
                    sphere->Transform.Position = glm::vec3(0.0f, 2.0f, 0.0f);
                    sphere->AddComponent<StaticMeshComponent>(PrimitiveType::Sphere);
                    sphere->AddComponent<RigidBodyComponent>();
                    m_SelectedActor = sphere;
                }
                if (ImGui::MenuItem("Directional Light")) {
                    auto light = scene.SpawnActor("Directional Light");
                    light->AddComponent<LightComponent>(LightType::Directional);
                    m_SelectedActor = light;
                }
                if (ImGui::MenuItem("Empty Actor")) {
                    m_SelectedActor = scene.SpawnActor("Empty Actor");
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        ImGui::EndChild();
        ImGui::End();
    }

} // namespace Engine
