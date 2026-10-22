/**
 * @file DetailsPanel.cpp
 * @brief Implementation of DetailsPanel dynamic reflection rendering.
 */

#include "DetailsPanel.hpp"
#include "core/Log.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "scene/AudioComponent.hpp"
#include "scene/PositionComponent.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/MovementComponent.hpp"
#include "scene/WidgetComponent.hpp"
#include "scene/CustomComponent.hpp"
#include "animation/SkeletalMeshComponent.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace Engine {

    void DetailsPanel::Render(Actor* actor) {
        ImGui::Begin("Details");

        if (!actor) {
            ImGui::TextDisabled("Select an Actor in the Outliner or Viewport to view details.");
            ImGui::End();
            return;
        }

        // 1. Actor Name & Header
        char nameBuf[128];
        std::strncpy(nameBuf, actor->GetName().c_str(), sizeof(nameBuf) - 1);
        nameBuf[sizeof(nameBuf) - 1] = '\0';
        if (ImGui::InputText("##ActorName", nameBuf, sizeof(nameBuf))) {
            actor->SetName(nameBuf);
        }

        ImGui::SameLine();
        ImGui::TextDisabled("(%s)", actor->GetClassName().data());

        // Quick open in 3D Actor Detail Window
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.75f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.55f, 0.88f, 1.0f));
        if (ImGui::Button("Open in 3D Actor Detail Window...", ImVec2(-1, 26.0f))) {
            if (OnOpenActorDetail) {
                OnOpenActorDetail(actor);
            }
        }
        ImGui::PopStyleColor(2);
        ImGui::Separator();

        // 2. Transform Section (UE5 Styled)
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawTransformControl("Location", actor->Transform.Position, 0.0f);
            DrawTransformControl("Rotation", actor->Transform.Rotation, 0.0f);
            DrawTransformControl("Scale",    actor->Transform.Scale,    1.0f);

            if (auto posComp = actor->GetComponent<PositionComponent>()) {
                posComp->Transform = actor->Transform;
            }
        }

        // 3. Components & Reflected Properties
        const auto& components = actor->GetComponents();
        Component* compToRemove = nullptr;
        for (const auto& comp : components) {
            std::string headerName = comp->GetName() + " (" + std::string(comp->GetClassName()) + ")";
            if (ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Remove Component")) {
                        compToRemove = comp.get();
                    }
                    ImGui::EndPopup();
                }
                if (auto coll = dynamic_cast<CollisionComponent*>(comp.get())) {
                    DrawUnityCollisionControls(*coll, actor);
                } else {
                    const ClassDescriptor* clsDesc = TypeRegistry::Get().FindClass(comp->GetClassName());
                    if (clsDesc) {
                        for (const auto& prop : clsDesc->GetProperties()) {
                            DrawReflectedProperty(prop, comp.get());
                        }
                    }
                }
            }
        }
        if (compToRemove) {
            actor->RemoveComponent(compToRemove);
        }

        // 4. Add Component Button
        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("+ Add Component", ImVec2(-1, 26))) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup")) {
            if (ImGui::MenuItem("Position Component")) {
                actor->AddComponent<PositionComponent>();
            }
            if (ImGui::MenuItem("Static Mesh Component")) {
                actor->AddComponent<StaticMeshComponent>();
            }
            if (ImGui::MenuItem("Collision Component")) {
                actor->AddComponent<CollisionComponent>();
            }
            if (ImGui::MenuItem("Physics Component")) {
                actor->AddComponent<PhysicsComponent>();
            }
            if (ImGui::MenuItem("Movement Component")) {
                actor->AddComponent<MovementComponent>();
            }
            if (ImGui::MenuItem("Light Component (Directional)")) {
                actor->AddComponent<LightComponent>(LightType::Directional);
            }
            if (ImGui::MenuItem("Light Component (Point)")) {
                actor->AddComponent<LightComponent>(LightType::Point);
            }
            if (ImGui::MenuItem("Audio Component")) {
                actor->AddComponent<AudioComponent>();
            }
            if (ImGui::MenuItem("Skeletal Mesh Component")) {
                actor->AddComponent<SkeletalMeshComponent>();
            }
            if (ImGui::MenuItem("Widget Component (HTML+CSS UI)")) {
                actor->AddComponent<WidgetComponent>();
            }

            ImGui::Separator();
            if (ImGui::BeginMenu("Custom Components")) {
                // Dynamically list registered components from TypeRegistry
                for (const auto& [name, desc] : TypeRegistry::Get().GetAllClasses()) {
                    if (name != "Actor" && name != "Character" && name != "Object") {
                        if (ImGui::MenuItem(name.c_str())) {
                            actor->AddComponent<CustomComponent>(name);
                            LOG_INFO("Attached custom component '{}' to Actor '{}'", name, actor->GetName());
                        }
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Attach Custom Component by Name...")) {
                m_bShowAttachByNameModal = true;
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Create New C++ Component Class...")) {
                m_bShowCreateComponentModal = true;
            }
            ImGui::EndPopup();
        }

        // Modal: Attach Custom Component by Name
        if (m_bShowAttachByNameModal) {
            ImGui::OpenPopup("Attach Custom Component###AttachByNameModal");
        }
        if (ImGui::BeginPopupModal("Attach Custom Component###AttachByNameModal", &m_bShowAttachByNameModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter Component Name to attach to '%s':", actor->GetName().c_str());
            ImGui::InputText("##AttachCompName", m_AttachComponentName, sizeof(m_AttachComponentName));
            ImGui::Spacing();

            if (ImGui::Button("Attach", ImVec2(100, 26))) {
                if (strlen(m_AttachComponentName) > 0) {
                    actor->AddComponent<CustomComponent>(m_AttachComponentName);
                    LOG_INFO("Attached custom component '{}' to Actor '{}'", m_AttachComponentName, actor->GetName());
                }
                m_bShowAttachByNameModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(80, 26))) {
                m_bShowAttachByNameModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Modal: Create C++ Component Class from Details Panel
        if (m_bShowCreateComponentModal) {
            ImGui::OpenPopup("Create C++ Component Class###DetailsCreateCompModal");
        }
        if (ImGui::BeginPopupModal("Create C++ Component Class###DetailsCreateCompModal", &m_bShowCreateComponentModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter new Component class name (derived from Engine::Component):");
            ImGui::InputText("##DetailsCompName", m_NewComponentName, sizeof(m_NewComponentName));
            ImGui::Spacing();

            if (ImGui::Button("Create Component Class", ImVec2(160, 0))) {
                if (m_Project && m_Project->GenerateComponentClass(m_NewComponentName)) {
                    LOG_INFO("Created custom C++ Component: {}", m_NewComponentName);
                }
                m_bShowCreateComponentModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                m_bShowCreateComponentModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void DetailsPanel::DrawTransformControl(const std::string& label, glm::vec3& values, float resetValue) {
        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 80.0f);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

        float lineHeight = GImGui->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        // X (Red)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("X", buttonSize)) values.x = resetValue;
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Y (Green)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.6f, 0.15f, 1.0f));
        if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Z (Blue)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.35f, 0.85f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.95f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.75f, 1.0f));
        if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f);
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();
    }

    void DetailsPanel::DrawReflectedProperty(const PropertyDescriptor& prop, void* instance) {
        ImGui::PushID(prop.GetName().c_str());

        switch (prop.GetType()) {
            case PropertyType::Float: {
                float& val = prop.GetValue<float>(instance);
                ImGui::DragFloat(prop.GetName().c_str(), &val, 0.05f);
                break;
            }
            case PropertyType::Int: {
                int& val = prop.GetValue<int>(instance);
                ImGui::DragInt(prop.GetName().c_str(), &val);
                break;
            }
            case PropertyType::Bool: {
                bool& val = prop.GetValue<bool>(instance);
                ImGui::Checkbox(prop.GetName().c_str(), &val);
                break;
            }
            case PropertyType::String: {
                std::string& str = prop.GetValue<std::string>(instance);
                char buf[256];
                std::strncpy(buf, str.c_str(), sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                if (ImGui::InputText(prop.GetName().c_str(), buf, sizeof(buf))) {
                    str = buf;
                }
                break;
            }
            case PropertyType::Vec3: {
                glm::vec3& vec = prop.GetValue<glm::vec3>(instance);
                ImGui::DragFloat3(prop.GetName().c_str(), &vec.x, 0.1f);
                break;
            }
            case PropertyType::Color3: {
                glm::vec3& col = prop.GetValue<glm::vec3>(instance);
                ImGui::ColorEdit3(prop.GetName().c_str(), &col.x);
                break;
            }
            case PropertyType::Color4: {
                glm::vec4& col = prop.GetValue<glm::vec4>(instance);
                ImGui::ColorEdit4(prop.GetName().c_str(), &col.x);
                break;
            }
            default:
                break;
        }

        if (!prop.GetTooltip().empty() && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", prop.GetTooltip().c_str());
        }

        ImGui::PopID();
    }

    void DetailsPanel::DrawUnityCollisionControls(CollisionComponent& coll, Actor* actor) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.13f, 0.15f, 0.18f, 1.0f));
        ImGui::BeginChild("UnityColliderBox", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.0f), "Unity-Style Collider Settings");
        ImGui::Separator();

        // 1. Edit Collider & Fit to Mesh buttons
        static bool s_EditColliderMode = false;
        if (s_EditColliderMode) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.65f, 0.35f, 1.0f));
        }
        if (ImGui::Button(s_EditColliderMode ? "[Edit Collider: ACTIVE]" : "Edit Collider", ImVec2(150.0f, 26.0f))) {
            s_EditColliderMode = !s_EditColliderMode;
        }
        if (s_EditColliderMode) {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        if (ImGui::Button("Fit to Mesh Geometry", ImVec2(160.0f, 26.0f))) {
            if (auto mesh = actor->GetComponent<StaticMeshComponent>()) {
                coll.FitToMesh(*mesh);
                LOG_INFO("Fitted collision bounds to mesh for Actor '{}'", actor->GetName());
            }
        }

        ImGui::Spacing();

        // 2. Is Trigger
        ImGui::Checkbox("Is Trigger", &coll.bIsTrigger);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("A Trigger detects overlap events without physical solid blocking.");
        }

        // 3. Shape Selection
        const char* shapes[] = { "Box Collider", "Sphere Collider" };
        int currentShape = static_cast<int>(coll.Shape);
        if (ImGui::Combo("Shape", &currentShape, shapes, IM_ARRAYSIZE(shapes))) {
            coll.Shape = static_cast<CollisionShape>(currentShape);
        }

        ImGui::Spacing();

        // 4. Center Offset
        DrawTransformControl("Center", coll.Offset, 0.0f);

        // 5. Size (Box Half Extents or Sphere Radius)
        if (coll.Shape == CollisionShape::Box) {
            DrawTransformControl("Size", coll.BoxHalfExtents, 0.5f);
        } else {
            ImGui::DragFloat("Radius", &coll.SphereRadius, 0.05f, 0.01f, 100.0f, "%.2f m");
        }

        ImGui::Spacing();
        ImGui::Separator();

        // 6. Physics Material Settings
        ImGui::SliderFloat("Bounciness (Restitution)", &coll.Restitution, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Dynamic Friction", &coll.Friction, 0.0f, 1.0f, "%.2f");

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

} // namespace Engine
