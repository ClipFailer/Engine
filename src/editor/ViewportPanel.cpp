/**
 * @file ViewportPanel.cpp
 * @brief Implementation of ViewportPanel with ImGuizmo integration and camera navigation.
 */

#include "ViewportPanel.hpp"
#include "scene/PositionComponent.hpp"
#include "scene/WidgetComponent.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "core/Log.hpp"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Engine {

    ViewportPanel::ViewportPanel(VulkanRenderer& renderer, Camera& camera, PhysicsWorld& physicsWorld)
        : m_Renderer(renderer), m_Camera(camera), m_PhysicsWorld(physicsWorld) {}

    void ViewportPanel::Render(Scene& scene, Actor* selectedActor) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        m_bIsHovered = ImGui::IsWindowHovered();
        m_bIsFocused = ImGui::IsWindowFocused();
        m_ViewportPos = ImGui::GetCursorScreenPos();

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (viewportPanelSize.x > 0.0f && viewportPanelSize.y > 0.0f) {
            if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y) {
                m_ViewportSize = viewportPanelSize;
                m_Renderer.ResizeViewport(static_cast<uint32>(viewportPanelSize.x), static_cast<uint32>(viewportPanelSize.y));
                m_Camera.SetViewportSize(viewportPanelSize.x, viewportPanelSize.y);
            }
        }

        // Render viewport image
        VkDescriptorSet textureId = m_Renderer.GetViewportTexture();
        if (textureId != VK_NULL_HANDLE) {
            ImGui::Image((ImTextureID)textureId, m_ViewportSize);
        }

        // Drag and drop target to spawn actor from Content Browser directly into 3D scene
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ACTOR_ASSET")) {
                std::string payloadData = (const char*)payload->Data;
                std::filesystem::path assetPath(payloadData);

                glm::vec3 spawnPos = m_Camera.GetPosition() + glm::vec3(0.0f, 0.0f, -4.0f);
                spawnPos.y = (std::max)(spawnPos.y, 1.0f);

                Actor* spawned = nullptr;
                if (assetPath.extension() == ".actor" && std::filesystem::exists(assetPath)) {
                    spawned = scene.SpawnActorFromAsset(assetPath, spawnPos);
                } else {
                    // Check if a .actor file with this stem name exists in Content directory
                    std::filesystem::path candidate = std::filesystem::current_path() / "Content" / (assetPath.stem().string() + ".actor");
                    if (std::filesystem::exists(candidate)) {
                        spawned = scene.SpawnActorFromAsset(candidate, spawnPos);
                    } else {
                        spawned = scene.SpawnActor(assetPath.stem().string());
                        spawned->Transform.Position = spawnPos;
                    }
                }

                if (spawned && OnActorSelected) {
                    OnActorSelected(spawned);
                }
                LOG_INFO("Spawned Actor '{}' into 3D Viewport via Drag-and-Drop", spawned ? spawned->GetName() : payloadData);
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_ASSET")) {
                std::string path = (const char*)payload->Data;
                if (selectedActor) {
                    auto mesh = selectedActor->GetComponent<StaticMeshComponent>();
                    if (!mesh) mesh = selectedActor->AddComponent<StaticMeshComponent>();
                    mesh->LoadModel(path);
                    LOG_INFO("Applied 3D Model '{}' to selected Actor '{}'", path, selectedActor->GetName());
                } else {
                    glm::vec3 spawnPos = m_Camera.GetPosition() + glm::vec3(0.0f, 0.0f, -4.0f);
                    spawnPos.y = (std::max)(spawnPos.y, 1.0f);
                    std::filesystem::path p(path);
                    auto spawned = scene.SpawnActor(p.stem().string());
                    spawned->Transform.Position = spawnPos;
                    if (auto mesh = spawned->GetComponent<StaticMeshComponent>()) {
                        mesh->LoadModel(path);
                    }
                    if (OnActorSelected) OnActorSelected(spawned);
                    LOG_INFO("Spawned Actor '{}' with model '{}' into 3D Viewport", spawned->GetName(), path);
                }
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
                std::string path = (const char*)payload->Data;
                if (selectedActor) {
                    auto mesh = selectedActor->GetComponent<StaticMeshComponent>();
                    if (!mesh) mesh = selectedActor->AddComponent<StaticMeshComponent>();
                    mesh->LoadTexture(path);
                    LOG_INFO("Applied Texture '{}' to selected Actor '{}'", path, selectedActor->GetName());
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Render 3D and Screen UI WidgetComponents
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        for (const auto& actor : scene.GetActors()) {
            if (!actor->bIsVisible) continue;
            if (auto widget = actor->GetComponent<WidgetComponent>()) {
                widget->Render(actor->Transform.Position, m_Camera, drawList, m_ViewportPos, m_ViewportSize);
            }
        }

        HandlePicking(scene);
        HandleCameraInput(scene, selectedActor);
        RenderToolbar();
        RenderGizmo(selectedActor);

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ViewportPanel::HandlePicking(const Scene& scene) {
        if (!m_bIsHovered || ImGuizmo::IsOver() || ImGuizmo::IsUsing()) return;

        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImVec2 mousePos = io.MousePos;
            float relX = mousePos.x - m_ViewportPos.x;
            float relY = mousePos.y - m_ViewportPos.y;

            if (relX >= 0.0f && relX <= m_ViewportSize.x && relY >= 0.0f && relY <= m_ViewportSize.y) {
                float ndcX = (relX / m_ViewportSize.x) * 2.0f - 1.0f;
                float ndcY = 1.0f - (relY / m_ViewportSize.y) * 2.0f;

                float aspect = m_ViewportSize.x / m_ViewportSize.y;
                glm::mat4 stdProj = glm::perspective(glm::radians(m_Camera.GetFOV()), aspect, m_Camera.GetNearClip(), m_Camera.GetFarClip());
                glm::mat4 invVP = glm::inverse(stdProj * m_Camera.GetViewMatrix());

                glm::vec4 nearPoint = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
                nearPoint /= nearPoint.w;
                glm::vec4 farPoint = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
                farPoint /= farPoint.w;

                glm::vec3 rayOrigin = m_Camera.GetPosition();
                glm::vec3 rayDir = glm::normalize(glm::vec3(farPoint - nearPoint));

                auto hit = m_PhysicsWorld.Raycast(scene, rayOrigin, rayDir, 500.0f);
                if (hit.bHit && hit.HitActor) {
                    if (OnActorSelected) OnActorSelected(hit.HitActor);
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && OnActorDoubleClicked) {
                        OnActorDoubleClicked(hit.HitActor);
                    }
                } else {
                    // Clicked in empty space -> deselect all objects
                    if (OnActorSelected) OnActorSelected(nullptr);
                }
            }
        }
    }

    void ViewportPanel::HandleCameraInput(Scene& scene, Actor* selectedActor) {
        if (!m_bIsHovered) return;

        ImGuiIO& io = ImGui::GetIO();
        float dt = io.DeltaTime;

        // Focus camera on selected actor (Unreal Engine style 'F' key)
        if (selectedActor && ImGui::IsKeyPressed(ImGuiKey_F)) {
            float focusDist = (std::max)(2.0f, glm::length(selectedActor->Transform.Scale) * 2.2f);
            m_Camera.FocusOn(selectedActor->Transform.Position, focusDist);
            LOG_INFO("Camera focused on actor '{}' (F key)", selectedActor->GetName());
        }

        // Delete selected actor on Delete key
        if (selectedActor && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            scene.DestroyActor(selectedActor);
            if (OnActorSelected) OnActorSelected(nullptr);
            return;
        }

        // Fly Camera (RMB held down) - intuitive non-inverted look direction
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            ImVec2 mouseDelta = io.MouseDelta;
            if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
                m_Camera.Rotate(-mouseDelta.y * 0.15f, -mouseDelta.x * 0.15f);
            }

            glm::vec3 moveDelta{0.0f};
            float speed = m_CameraSpeed * (io.KeyShift ? 2.5f : 1.0f) * dt;

            if (ImGui::IsKeyDown(ImGuiKey_W)) moveDelta.z += speed;
            if (ImGui::IsKeyDown(ImGuiKey_S)) moveDelta.z -= speed;
            if (ImGui::IsKeyDown(ImGuiKey_D)) moveDelta.x += speed;
            if (ImGui::IsKeyDown(ImGuiKey_A)) moveDelta.x -= speed;
            if (ImGui::IsKeyDown(ImGuiKey_E)) moveDelta.y += speed;
            if (ImGui::IsKeyDown(ImGuiKey_Q)) moveDelta.y -= speed;

            m_Camera.MoveLocal(moveDelta);
        }

        // Zoom via mouse wheel
        if (io.MouseWheel != 0.0f) {
            m_Camera.MoveLocal(glm::vec3(0.0f, 0.0f, io.MouseWheel * m_CameraSpeed * 0.5f));
        }

        // Shortcut keys for Gizmo when Viewport focused
        if (m_bIsFocused && !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            if (ImGui::IsKeyPressed(ImGuiKey_W)) m_GizmoOperation = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_E)) m_GizmoOperation = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R)) m_GizmoOperation = ImGuizmo::SCALE;
        }
    }

    void ViewportPanel::RenderToolbar() {
        ImGui::SetCursorPos(ImVec2(10.0f, 30.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.14f, 0.85f));
        ImGui::BeginChild("ViewportToolbar", ImVec2(340.0f, 34.0f), true, ImGuiWindowFlags_NoScrollbar);

        if (ImGui::RadioButton("Translate (W)", m_GizmoOperation == ImGuizmo::TRANSLATE)) {
            m_GizmoOperation = ImGuizmo::TRANSLATE;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate (E)", m_GizmoOperation == ImGuizmo::ROTATE)) {
            m_GizmoOperation = ImGuizmo::ROTATE;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale (R)", m_GizmoOperation == ImGuizmo::SCALE)) {
            m_GizmoOperation = ImGuizmo::SCALE;
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void ViewportPanel::RenderGizmo(Actor* selectedActor) {
        if (!selectedActor) return;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        // Exact viewport rectangle matching offscreen render buffer image
        ImGuizmo::SetRect(m_ViewportPos.x, m_ViewportPos.y, m_ViewportSize.x, m_ViewportSize.y);

        glm::mat4 cameraView = m_Camera.GetViewMatrix();
        glm::mat4 cameraProj = m_Camera.GetProjectionMatrix();
        // Undo Vulkan Y flip for ImGuizmo
        cameraProj[1][1] *= -1.0f;

        glm::mat4 transform = selectedActor->Transform.GetMatrix();

        ImGuizmo::Manipulate(
            glm::value_ptr(cameraView),
            glm::value_ptr(cameraProj),
            m_GizmoOperation,
            m_GizmoMode,
            glm::value_ptr(transform)
        );

        if (ImGuizmo::IsUsing()) {
            glm::vec3 translation, scale;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::quat orientation;
            glm::decompose(transform, scale, orientation, translation, skew, perspective);

            selectedActor->Transform.Position = translation;
            selectedActor->Transform.Rotation = glm::degrees(glm::eulerAngles(orientation));
            selectedActor->Transform.Scale = scale;

            if (auto posComp = selectedActor->GetComponent<PositionComponent>()) {
                posComp->Transform = selectedActor->Transform;
            }
        }
    }

} // namespace Engine
