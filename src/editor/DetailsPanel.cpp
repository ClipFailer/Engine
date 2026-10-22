/**
 * @file DetailsPanel.cpp
 * @brief Implementation of DetailsPanel dynamic reflection rendering.
 */

#include "DetailsPanel.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "scene/AudioComponent.hpp"
#include "scene/PositionComponent.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/MovementComponent.hpp"
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
        ImGui::Separator();

        // 2. Transform Section (UE5 Styled)
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawTransformControl("Location", actor->Transform.Position, 0.0f);
            DrawTransformControl("Rotation", actor->Transform.Rotation, 0.0f);
            DrawTransformControl("Scale",    actor->Transform.Scale,    1.0f);
        }

        // 3. Components & Reflected Properties
        const auto& components = actor->GetComponents();
        for (const auto& comp : components) {
            std::string headerName = comp->GetName() + " (" + std::string(comp->GetClassName()) + ")";
            if (ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                const ClassDescriptor* clsDesc = TypeRegistry::Get().FindClass(comp->GetClassName());
                if (clsDesc) {
                    for (const auto& prop : clsDesc->GetProperties()) {
                        DrawReflectedProperty(prop, comp.get());
                    }
                }
            }
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

} // namespace Engine
