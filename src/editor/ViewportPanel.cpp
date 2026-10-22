/**
 * @file ViewportPanel.cpp
 * @brief Implementation of ViewportPanel with ImGuizmo integration and camera navigation.
 */

#include "ViewportPanel.hpp"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Engine {

    ViewportPanel::ViewportPanel(VulkanRenderer& renderer, Camera& camera)
        : m_Renderer(renderer), m_Camera(camera) {}

    void ViewportPanel::Render(Scene& scene, Actor* selectedActor) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        m_bIsHovered = ImGui::IsWindowHovered();
        m_bIsFocused = ImGui::IsWindowFocused();

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

        HandleCameraInput();
        RenderToolbar();
        RenderGizmo(selectedActor);

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ViewportPanel::HandleCameraInput() {
        if (!m_bIsHovered) return;

        ImGuiIO& io = ImGui::GetIO();
        float dt = io.DeltaTime;

        // Fly Camera (RMB held down)
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            ImVec2 mouseDelta = io.MouseDelta;
            if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
                m_Camera.Rotate(mouseDelta.y * 0.15f, mouseDelta.x * 0.15f);
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

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImGuizmo::SetRect(windowPos.x, windowPos.y, m_ViewportSize.x, m_ViewportSize.y);

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
        }
    }

} // namespace Engine
