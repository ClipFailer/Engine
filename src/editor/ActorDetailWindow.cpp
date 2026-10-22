/**
 * @file ActorDetailWindow.cpp
 * @brief Implementation of ActorDetailWindow for mesh selection, tags, and collision fitting.
 */

#include "ActorDetailWindow.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/PositionComponent.hpp"
#include "reflection/Reflection.hpp"
#include <imgui.h>

namespace Engine {

    void ActorDetailWindow::Render() {
        if (!m_bIsOpen || !m_TargetActor) return;

        ImGui::SetNextWindowSize(ImVec2(520, 640), ImGuiCond_FirstUseEver);
        std::string title = "Actor Details - " + m_TargetActor->GetName() + "###ActorDetailWindow";

        if (ImGui::Begin(title.c_str(), &m_bIsOpen)) {
            // 1. General Info & Tag
            if (ImGui::CollapsingHeader("General & Tag", ImGuiTreeNodeFlags_DefaultOpen)) {
                char nameBuf[128];
                std::strncpy(nameBuf, m_TargetActor->GetName().c_str(), sizeof(nameBuf) - 1);
                nameBuf[sizeof(nameBuf) - 1] = '\0';
                if (ImGui::InputText("Actor Name", nameBuf, sizeof(nameBuf))) {
                    m_TargetActor->SetName(nameBuf);
                }

                char tagBuf[64];
                std::strncpy(tagBuf, m_TargetActor->Tag.c_str(), sizeof(tagBuf) - 1);
                tagBuf[sizeof(tagBuf) - 1] = '\0';
                if (ImGui::InputText("Tag", tagBuf, sizeof(tagBuf))) {
                    m_TargetActor->Tag = tagBuf;
                }

                ImGui::SameLine();
                if (ImGui::BeginCombo("##TagPresets", "Presets")) {
                    const char* presets[] = {"Player", "Enemy", "Obstacle", "Pickup", "Trigger", "Environment"};
                    for (const char* p : presets) {
                        if (ImGui::Selectable(p)) {
                            m_TargetActor->Tag = p;
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Checkbox("Visible in World", &m_TargetActor->bIsVisible);
            }

            // 2. Mesh Configuration
            if (auto meshComp = m_TargetActor->GetComponent<StaticMeshComponent>()) {
                if (ImGui::CollapsingHeader("Static Mesh Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                    const char* meshNames[] = {"Cube", "Sphere", "Plane", "Custom"};
                    int currentMeshIndex = static_cast<int>(meshComp->MeshType);
                    if (ImGui::Combo("Mesh Primitive", &currentMeshIndex, meshNames, IM_ARRAYSIZE(meshNames))) {
                        meshComp->MeshType = static_cast<PrimitiveType>(currentMeshIndex);
                    }

                    ImGui::ColorEdit4("Base Albedo Color", &meshComp->BaseColor.x);
                    ImGui::SliderFloat("Metallic", &meshComp->Metallic, 0.0f, 1.0f);
                    ImGui::SliderFloat("Roughness", &meshComp->Roughness, 0.0f, 1.0f);
                    ImGui::Checkbox("Cast Dynamic Shadows", &meshComp->bCastShadows);
                }
            }

            // 3. Collision Configuration & Bounds Fitting
            if (auto collComp = m_TargetActor->GetComponent<CollisionComponent>()) {
                if (ImGui::CollapsingHeader("Collision Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                    const char* shapeNames[] = {"Box Collider", "Sphere Collider"};
                    int currentShape = static_cast<int>(collComp->Shape);
                    if (ImGui::Combo("Collider Shape", &currentShape, shapeNames, IM_ARRAYSIZE(shapeNames))) {
                        collComp->Shape = static_cast<CollisionShape>(currentShape);
                    }

                    if (collComp->Shape == CollisionShape::Box) {
                        ImGui::DragFloat3("Box Half Extents", &collComp->BoxHalfExtents.x, 0.05f, 0.05f, 50.0f);
                    } else {
                        ImGui::DragFloat("Sphere Radius", &collComp->SphereRadius, 0.05f, 0.05f, 50.0f);
                    }

                    ImGui::DragFloat3("Center Offset", &collComp->Offset.x, 0.05f);
                    ImGui::SliderFloat("Restitution (Bounce)", &collComp->Restitution, 0.0f, 1.0f);
                    ImGui::SliderFloat("Friction", &collComp->Friction, 0.0f, 1.0f);
                    ImGui::Checkbox("Is Trigger (Overlap Only)", &collComp->bIsTrigger);

                    ImGui::Spacing();
                    if (auto mesh = m_TargetActor->GetComponent<StaticMeshComponent>()) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.55f, 0.35f, 1.0f));
                        if (ImGui::Button("Fit Collision to Mesh", ImVec2(-1, 28))) {
                            collComp->FitToMesh(*mesh);
                        }
                        ImGui::PopStyleColor();
                    }
                }
            }

            // 4. Physics Configuration
            if (auto physComp = m_TargetActor->GetComponent<PhysicsComponent>()) {
                if (ImGui::CollapsingHeader("Physics Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat("Mass (kg)", &physComp->Mass, 0.1f, 0.0f, 1000.0f);
                    ImGui::Checkbox("Use Gravity", &physComp->bUseGravity);
                    ImGui::Checkbox("Is Static (Immovable)", &physComp->bIsStatic);
                    ImGui::DragFloat3("Linear Velocity", &physComp->LinearVelocity.x, 0.1f);
                    ImGui::SliderFloat("Linear Damping", &physComp->LinearDamping, 0.0f, 1.0f);
                }
            }

            // 5. All Components List & Reflection Properties
            if (ImGui::CollapsingHeader("All Components", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (const auto& comp : m_TargetActor->GetComponents()) {
                    ImGui::BulletText("%s (%s)", comp->GetName().c_str(), comp->GetClassName().data());
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Button("Close", ImVec2(100, 26))) {
                m_bIsOpen = false;
            }
        }
        ImGui::End();
    }

} // namespace Engine
