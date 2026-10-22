/**
 * @file EditorApp.hpp
 * @brief Unreal Engine 5 styled desktop editor application hosting viewport, panels, and simulation.
 */

#pragma once

#include "rhi/VulkanContext.hpp"
#include "rhi/VulkanSwapChain.hpp"
#include "rhi/VulkanRenderer.hpp"
#include "audio/AudioEngine.hpp"
#include "physics/PhysicsWorld.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "project/Project.hpp"
#include "ViewportPanel.hpp"
#include "OutlinerPanel.hpp"
#include "DetailsPanel.hpp"
#include "ContentBrowserPanel.hpp"
#include "ConsolePanel.hpp"
#include "ActorDetailWindow.hpp"
#include "SettingsDialogs.hpp"
#include <memory>

namespace Engine {

    /**
     * @class EditorApp
     * @brief Central editor application managing window lifecycle, docking UI, and subsystem loops.
     */
    class EditorApp {
    public:
        EditorApp();
        ~EditorApp();

        /**
         * @brief Initializes GLFW window, Vulkan RHI, ImGui docking, and engine subsystems.
         * @return True if successfully initialized.
         */
        bool Initialize();

        /**
         * @brief Runs the main application frame loop until window close requested.
         */
        void Run();

        /**
         * @brief Shuts down all subsystems and cleans up Vulkan resources.
         */
        void Shutdown();

    private:
        void InitImGui();
        void ShutdownImGui();
        void RenderUI();
        void RenderMainMenuBar();
        void RenderTopToolbar();
        void RenderNewProjectModal();

        GLFWwindow* m_Window{nullptr};
        VulkanContext m_VulkanContext;
        std::unique_ptr<VulkanSwapChain> m_SwapChain;
        std::unique_ptr<VulkanRenderer> m_Renderer;
        std::unique_ptr<AudioEngine> m_AudioEngine;
        std::unique_ptr<PhysicsWorld> m_PhysicsWorld;
        std::unique_ptr<Scene> m_Scene;
        std::unique_ptr<Camera> m_Camera;
        std::unique_ptr<Project> m_Project;

        std::unique_ptr<ViewportPanel> m_ViewportPanel;
        std::unique_ptr<OutlinerPanel> m_OutlinerPanel;
        std::unique_ptr<DetailsPanel> m_DetailsPanel;
        std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;
        std::unique_ptr<ConsolePanel> m_ConsolePanel;
        std::unique_ptr<ActorDetailWindow> m_ActorDetailWindow;
        std::unique_ptr<SettingsDialogs> m_SettingsDialogs;

        bool m_bShowNewProjectModal{false};
        char m_NewProjectName[64]{"MyNewGame"};
        char m_NewProjectFolder[256]{"C:/Projects/Vulkan/MyNewGame"};

        VkDescriptorPool m_ImGuiDescriptorPool{VK_NULL_HANDLE};
        std::vector<VkCommandBuffer> m_CommandBuffers;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        uint32 m_CurrentFrame{0};
        const int MAX_FRAMES_IN_FLIGHT{2};
        bool m_bIsRunning{true};
    };

} // namespace Engine
