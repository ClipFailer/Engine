/**
 * @file EditorApp.cpp
 * @brief Implementation of EditorApp window, docking UI, and main loop.
 */

#include "EditorApp.hpp"
#include "UE5Theme.hpp"
#include "core/Log.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "module/ModuleManager.hpp"
#include "ui/UIModule.hpp"
#include "animation/AnimationModule.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <chrono>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Engine {

    EditorApp::EditorApp() = default;

    EditorApp::~EditorApp() {
        Shutdown();
    }

    bool EditorApp::Initialize() {
#if defined(_WIN32)
        // Hide Windows command prompt window on startup for clean desktop editor experience
        HWND consoleWnd = GetConsoleWindow();
        if (consoleWnd) {
            ShowWindow(consoleWnd, SW_HIDE);
        }
#endif

        // 1. Initialize GLFW
        if (!glfwInit()) {
            LOG_ERROR("Failed to initialize GLFW");
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_Window = glfwCreateWindow(1600, 900, "Vulkan 3D Engine - Unreal Editor 5 UX", nullptr, nullptr);
        if (!m_Window) {
            LOG_ERROR("Failed to create GLFW window");
            return false;
        }

        // 2. Vulkan Context & SwapChain
        if (!m_VulkanContext.Initialize(m_Window)) {
            LOG_ERROR("Failed to initialize VulkanContext");
            return false;
        }

        m_SwapChain = std::make_unique<VulkanSwapChain>(m_VulkanContext, m_Window);
        if (!m_SwapChain->Initialize()) {
            LOG_ERROR("Failed to initialize VulkanSwapChain");
            return false;
        }

        // 3. Audio & Physics
        m_AudioEngine = std::make_unique<AudioEngine>();
        m_AudioEngine->Initialize();

        m_PhysicsWorld = std::make_unique<PhysicsWorld>();

        // 4. Register Engine Modules
        ModuleManager::Get().RegisterModule<UIModule>();
        ModuleManager::Get().RegisterModule<AnimationModule>();

        // 5. Project, Scene, Camera
        m_Project = std::make_unique<Project>();
        m_Scene = std::make_unique<Scene>();
        m_Camera = std::make_unique<Camera>();
        m_Camera->SetPosition(glm::vec3(0.0f, 4.0f, 10.0f));

        // 6. ImGui Initialization
        InitImGui();

        // 7. Vulkan Forward Renderer
        m_Renderer = std::make_unique<VulkanRenderer>(m_VulkanContext);
        if (!m_Renderer->Initialize()) {
            LOG_ERROR("Failed to initialize VulkanRenderer");
            return false;
        }

        // 8. Panels & Windows
        m_ViewportPanel = std::make_unique<ViewportPanel>(*m_Renderer, *m_Camera, *m_PhysicsWorld);
        m_OutlinerPanel = std::make_unique<OutlinerPanel>();
        m_DetailsPanel = std::make_unique<DetailsPanel>();
        m_ContentBrowserPanel = std::make_unique<ContentBrowserPanel>(*m_Project);
        m_ConsolePanel = std::make_unique<ConsolePanel>();
        m_ActorDetailWindow = std::make_unique<ActorDetailWindow>();
        m_SettingsDialogs = std::make_unique<SettingsDialogs>();

        // Wire selection and double-click callbacks
        m_ViewportPanel->OnActorSelected = [this](Actor* a) {
            m_OutlinerPanel->SetSelectedActor(a);
        };
        m_ViewportPanel->OnActorDoubleClicked = [this](Actor* a) {
            m_ActorDetailWindow->Open(a);
        };
        m_OutlinerPanel->OnActorDoubleClicked = [this](Actor* a) {
            m_ActorDetailWindow->Open(a);
        };

        // 9. Command Buffers & Sync Objects
        m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_VulkanContext.GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32>(m_CommandBuffers.size());
        vkAllocateCommandBuffers(m_VulkanContext.GetDevice(), &allocInfo, m_CommandBuffers.data());

        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkCreateSemaphore(m_VulkanContext.GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]);
            vkCreateSemaphore(m_VulkanContext.GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
            vkCreateFence(m_VulkanContext.GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]);
        }

        LOG_INFO("EditorApp initialized successfully");
        return true;
    }

    void EditorApp::InitImGui() {
        // Create Descriptor Pool for ImGui
        VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
        poolInfo.poolSizeCount = static_cast<uint32>(IM_ARRAYSIZE(poolSizes));
        poolInfo.pPoolSizes = poolSizes;

        vkCreateDescriptorPool(m_VulkanContext.GetDevice(), &poolInfo, nullptr, &m_ImGuiDescriptorPool);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Apply Unreal Engine 5 Dark Slate Theme
        UE5Theme::Apply();

        ImGui_ImplGlfw_InitForVulkan(m_Window, true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = m_VulkanContext.GetInstance();
        initInfo.PhysicalDevice = m_VulkanContext.GetPhysicalDevice();
        initInfo.Device = m_VulkanContext.GetDevice();
        initInfo.QueueFamily = m_VulkanContext.GetQueueFamilies().GraphicsFamily.value();
        initInfo.Queue = m_VulkanContext.GetGraphicsQueue();
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = m_ImGuiDescriptorPool;
        initInfo.PipelineInfoMain.Subpass = 0;
        initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.PipelineInfoMain.RenderPass = m_SwapChain->GetRenderPass();
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = m_SwapChain->GetImageCount();

        ImGui_ImplVulkan_Init(&initInfo);
    }

    void EditorApp::ShutdownImGui() {
        vkDeviceWaitIdle(m_VulkanContext.GetDevice());
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (m_ImGuiDescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_VulkanContext.GetDevice(), m_ImGuiDescriptorPool, nullptr);
            m_ImGuiDescriptorPool = VK_NULL_HANDLE;
        }
    }

    void EditorApp::Shutdown() {
        VkDevice device = m_VulkanContext.GetDevice();
        if (device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(device);

            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                if (m_InFlightFences[i] != VK_NULL_HANDLE) vkDestroyFence(device, m_InFlightFences[i], nullptr);
                if (m_ImageAvailableSemaphores[i] != VK_NULL_HANDLE) vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
                if (m_RenderFinishedSemaphores[i] != VK_NULL_HANDLE) vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
            }
        }

        m_Renderer.reset();
        ShutdownImGui();
        m_SwapChain.reset();
        m_AudioEngine.reset();
        ModuleManager::Get().Shutdown();
        m_VulkanContext.Shutdown();

        if (m_Window) {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }
        glfwTerminate();
    }

    void EditorApp::Run() {
        auto lastTime = std::chrono::high_resolution_clock::now();

        while (!glfwWindowShouldClose(m_Window) && m_bIsRunning) {
            glfwPollEvents();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Update Modules
            ModuleManager::Get().UpdateModules(deltaTime);

            // Physics and Simulation Step
            if (m_Scene->GetState() == SimulationState::Playing) {
                m_PhysicsWorld->Step(*m_Scene, deltaTime);
                m_Scene->Tick(deltaTime);
            }

            // Update Audio Listener
            m_AudioEngine->UpdateListener(m_Camera->GetPosition(), m_Camera->GetPosition(), glm::vec3(0, 1, 0));

            // Offscreen 3D Scene Render into Viewport Texture
            m_Renderer->RenderScene(*m_Scene, *m_Camera);

            // Wait for previous frame fence
            VkDevice device = m_VulkanContext.GetDevice();
            vkWaitForFences(device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

            uint32 imageIndex;
            VkResult result = m_SwapChain->AcquireNextImage(&imageIndex, m_ImageAvailableSemaphores[m_CurrentFrame]);
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                m_SwapChain->Recreate();
                continue;
            }

            vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]);

            // Start ImGui Frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGuizmo::BeginFrame();

            RenderUI();

            ImGui::Render();

            // Record Swapchain command buffer
            VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
            vkResetCommandBuffer(cmd, 0);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            vkBeginCommandBuffer(cmd, &beginInfo);

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
            renderPassInfo.framebuffer = m_SwapChain->GetFramebuffers()[imageIndex];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_SwapChain->GetExtent();

            VkClearValue clearColor = {{{0.1f, 0.1f, 0.12f, 1.0f}}};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
            vkCmdEndRenderPass(cmd);

            vkEndCommandBuffer(cmd);

            // Submit queue
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;

            VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            vkQueueSubmit(m_VulkanContext.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);

            m_SwapChain->Present(imageIndex, m_RenderFinishedSemaphores[m_CurrentFrame]);

            m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        vkDeviceWaitIdle(m_VulkanContext.GetDevice());
    }

    void EditorApp::RenderUI() {
        // Fullscreen DockSpace
        ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("MasterDockSpace", nullptr, windowFlags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);

        RenderMainMenuBar();
        RenderTopToolbar();

        // Render all panels
        m_ViewportPanel->Render(*m_Scene, m_OutlinerPanel->GetSelectedActor());
        m_OutlinerPanel->Render(*m_Scene);
        m_DetailsPanel->Render(m_OutlinerPanel->GetSelectedActor());
        m_ContentBrowserPanel->Render(*m_Scene);
        m_ConsolePanel->Render();

        m_ActorDetailWindow->Render();
        m_SettingsDialogs->Render(*m_Project, *m_PhysicsWorld);
        RenderNewProjectModal();

        ImGui::End();
    }

    void EditorApp::RenderMainMenuBar() {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Project...")) {
                    m_bShowNewProjectModal = true;
                }
                if (ImGui::MenuItem("Save Scene")) {
                    m_Scene->SaveToFile("DefaultScene.json");
                }
                if (ImGui::MenuItem("Exit")) {
                    m_bIsRunning = false;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Project Settings...")) {
                    m_SettingsDialogs->OpenProjectSettings();
                }
                if (ImGui::MenuItem("Engine Settings...")) {
                    m_SettingsDialogs->OpenEngineSettings();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Actor")) {
                if (ImGui::MenuItem("Spawn Cube")) {
                    auto a = m_Scene->SpawnActor("Cube");
                    m_OutlinerPanel->SetSelectedActor(a);
                }
                if (ImGui::MenuItem("Spawn Sphere")) {
                    auto a = m_Scene->SpawnActor("Sphere");
                    if (auto mesh = a->GetComponent<StaticMeshComponent>()) {
                        mesh->MeshType = PrimitiveType::Sphere;
                    }
                    m_OutlinerPanel->SetSelectedActor(a);
                }
                if (ImGui::MenuItem("Spawn Directional Light")) {
                    auto a = m_Scene->SpawnActor("Sun");
                    a->AddComponent<LightComponent>(LightType::Directional);
                    m_OutlinerPanel->SetSelectedActor(a);
                }
                if (ImGui::MenuItem("Spawn Point Light")) {
                    auto a = m_Scene->SpawnActor("Point Light");
                    a->Transform.Position = glm::vec3(0.0f, 3.0f, 0.0f);
                    auto pl = a->AddComponent<LightComponent>(LightType::Point);
                    pl->Radius = 12.0f;
                    pl->Intensity = 2.0f;
                    m_OutlinerPanel->SetSelectedActor(a);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Open Actor Detail Window", nullptr, false, m_OutlinerPanel->GetSelectedActor() != nullptr)) {
                    m_ActorDetailWindow->Open(m_OutlinerPanel->GetSelectedActor());
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Build")) {
                if (ImGui::MenuItem("Build Game Code (Clang)")) {
                    m_Project->BuildProject();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void EditorApp::RenderNewProjectModal() {
        if (m_bShowNewProjectModal) {
            ImGui::OpenPopup("Create New Game Project###NewProjectModal");
        }

        if (ImGui::BeginPopupModal("Create New Game Project###NewProjectModal", &m_bShowNewProjectModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter Project Name:");
            if (ImGui::InputText("##ProjName", m_NewProjectName, sizeof(m_NewProjectName))) {
                std::string folder = "C:/Projects/Vulkan/" + std::string(m_NewProjectName);
                std::strncpy(m_NewProjectFolder, folder.c_str(), sizeof(m_NewProjectFolder) - 1);
            }

            ImGui::Text("Project Destination Folder:");
            ImGui::InputText("##ProjFolder", m_NewProjectFolder, sizeof(m_NewProjectFolder));

            ImGui::Spacing();
            ImGui::Separator();

            if (ImGui::Button("Create & Open Project", ImVec2(180, 28))) {
                m_Project = Project::CreateNew(std::filesystem::path(m_NewProjectFolder), m_NewProjectName);
                m_ContentBrowserPanel->SetProject(*m_Project);
                LOG_INFO("Opened newly created project: {} at {}", m_NewProjectName, m_NewProjectFolder);
                m_bShowNewProjectModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(80, 28))) {
                m_bShowNewProjectModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void EditorApp::RenderTopToolbar() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 2.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
        ImGui::Begin("##TopToolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        float btnWidth = 70.0f;
        float btnHeight = 26.0f;
        float avail = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX((avail - (btnWidth * 3.0f + 16.0f)) * 0.5f);

        SimulationState state = m_Scene->GetState();

        // 1. Play Button (Green)
        bool isPlaying = (state == SimulationState::Playing);
        if (isPlaying) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.65f, 0.25f, 1.0f));
        if (ImGui::Button(isPlaying ? "Simulating" : "Play", ImVec2(btnWidth, btnHeight))) {
            if (state == SimulationState::Stopped) {
                m_Scene->StartSimulation();
            } else if (state == SimulationState::Paused) {
                m_Scene->ResumeSimulation();
            }
        }
        if (isPlaying) ImGui::PopStyleColor();

        ImGui::SameLine();

        // 2. Pause Button (Yellow)
        bool isPaused = (state == SimulationState::Paused);
        if (isPaused) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.65f, 0.15f, 1.0f));
        if (ImGui::Button("Pause", ImVec2(btnWidth, btnHeight))) {
            if (state == SimulationState::Playing) {
                m_Scene->PauseSimulation();
            }
        }
        if (isPaused) ImGui::PopStyleColor();

        ImGui::SameLine();

        // 3. Stop Button (Red)
        if (ImGui::Button("Stop", ImVec2(btnWidth, btnHeight))) {
            m_Scene->StopSimulation();
        }

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

} // namespace Engine
