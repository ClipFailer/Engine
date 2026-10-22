/**
 * @file SettingsDialogs.cpp
 * @brief Implementation of Project Settings and Engine Settings modals.
 */

#include "SettingsDialogs.hpp"
#include <imgui.h>

namespace Engine {

    void SettingsDialogs::Render(Project& project, PhysicsWorld& physicsWorld) {
        // 1. Project Settings Modal
        if (m_bShowProjectSettings) {
            ImGui::OpenPopup("Project Settings###ProjectSettingsModal");
        }

        if (ImGui::BeginPopupModal("Project Settings###ProjectSettingsModal", &m_bShowProjectSettings, ImGuiWindowFlags_AlwaysAutoResize)) {
            ProjectConfig config = project.GetConfig();

            char nameBuf[128];
            std::strncpy(nameBuf, config.Name.c_str(), sizeof(nameBuf) - 1);
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            if (ImGui::InputText("Project Name", nameBuf, sizeof(nameBuf))) {
                config.Name = nameBuf;
            }

            char verBuf[64];
            std::strncpy(verBuf, config.Version.c_str(), sizeof(verBuf) - 1);
            verBuf[sizeof(verBuf) - 1] = '\0';
            if (ImGui::InputText("Version", verBuf, sizeof(verBuf))) {
                config.Version = verBuf;
            }

            char sceneBuf[256];
            std::strncpy(sceneBuf, config.StartScene.c_str(), sizeof(sceneBuf) - 1);
            sceneBuf[sizeof(sceneBuf) - 1] = '\0';
            if (ImGui::InputText("Start Scene", sceneBuf, sizeof(sceneBuf))) {
                config.StartScene = sceneBuf;
            }

            ImGui::Separator();
            ImGui::Text("Global Physics Environment:");
            ImGui::DragFloat3("World Gravity", &physicsWorld.Gravity.x, 0.1f);

            ImGui::Spacing();
            if (ImGui::Button("Save Settings", ImVec2(120, 26))) {
                project.Save();
                m_bShowProjectSettings = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(80, 26))) {
                m_bShowProjectSettings = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // 2. Engine Settings Modal
        if (m_bShowEngineSettings) {
            ImGui::OpenPopup("Engine Settings###EngineSettingsModal");
        }

        if (ImGui::BeginPopupModal("Engine Settings###EngineSettingsModal", &m_bShowEngineSettings, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Editor & Viewport Preferences:");
            ImGui::SliderFloat("Camera Speed", &m_CameraBaseSpeed, 1.0f, 20.0f);
            ImGui::SliderFloat("Mouse Sensitivity", &m_MouseSensitivity, 0.05f, 1.0f);
            ImGui::Checkbox("VSync Enabled", &m_bVSync);

            ImGui::Separator();
            ImGui::Text("Active Graphics Backend: Vulkan 1.4 Dynamic Offscreen");
            ImGui::Text("Compiler: Clang 23 (LLVM) + Ninja");

            ImGui::Spacing();
            if (ImGui::Button("OK", ImVec2(80, 26))) {
                m_bShowEngineSettings = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

} // namespace Engine
