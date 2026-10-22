/**
 * @file ViewportPanel.hpp
 * @brief Viewport dock panel with interactive 3D camera navigation and ImGuizmo manipulation.
 */

#pragma once

#include <imgui.h>
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "physics/PhysicsWorld.hpp"
#include "rhi/VulkanRenderer.hpp"
#include <ImGuizmo.h>
#include <functional>

namespace Engine {

    /**
     * @class ViewportPanel
     * @brief Displays the 3D level viewport with Unreal-style camera controls and manipulation gizmo.
     */
    class ViewportPanel {
    public:
        ViewportPanel(VulkanRenderer& renderer, Camera& camera, PhysicsWorld& physicsWorld);

        /**
         * @brief Renders the viewport panel GUI and processes input navigation.
         * @param scene Active level scene.
         * @param selectedActor Currently selected actor in the editor.
         */
        void Render(Scene& scene, Actor* selectedActor);

        [[nodiscard]] bool IsHovered() const noexcept { return m_bIsHovered; }
        [[nodiscard]] bool IsFocused() const noexcept { return m_bIsFocused; }

        std::function<void(Actor*)> OnActorSelected;
        std::function<void(Actor*)> OnActorDoubleClicked;

    private:
        void HandleCameraInput(Scene& scene, Actor* selectedActor);
        void RenderToolbar();
        void RenderGizmo(Actor* selectedActor);
        void HandlePicking(const Scene& scene);

        VulkanRenderer& m_Renderer;
        Camera& m_Camera;
        PhysicsWorld& m_PhysicsWorld;

        bool m_bIsHovered{false};
        bool m_bIsFocused{false};
        ImVec2 m_ViewportSize{1280.0f, 720.0f};
        ImVec2 m_ViewportPos{0.0f, 0.0f};

        ImGuizmo::OPERATION m_GizmoOperation{ImGuizmo::TRANSLATE};
        ImGuizmo::MODE m_GizmoMode{ImGuizmo::WORLD};
        float m_CameraSpeed{5.0f};
    };

} // namespace Engine
