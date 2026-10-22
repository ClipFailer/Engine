/**
 * @file SettingsDialogs.cpp
 * @brief Implementation of Project Settings and Engine Settings modals.
 */

#include "SettingsDialogs.hpp"
#include "rhi/RenderSettings.hpp"
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
            RenderSettings& rs = RenderSettings::Get();

            if (ImGui::CollapsingHeader("Graphics & Quality Presets", ImGuiTreeNodeFlags_DefaultOpen)) {
                const char* presets[] = { "Low", "Medium", "High", "Ultra", "Cinematic" };
                int currentPreset = static_cast<int>(rs.QualityPreset);
                if (ImGui::Combo("Quality Preset", &currentPreset, presets, IM_ARRAYSIZE(presets))) {
                    rs.ApplyPreset(static_cast<RenderQualityPreset>(currentPreset));
                }

                const char* aaModes[] = { "None", "FXAA (Fast Approximate)", "MSAA 2x (Multi-Sample)", "MSAA 4x (Multi-Sample)" };
                int currentAA = static_cast<int>(rs.AAMode);
                if (ImGui::Combo("Anti-Aliasing", &currentAA, aaModes, IM_ARRAYSIZE(aaModes))) {
                    rs.AAMode = static_cast<AntiAliasingMode>(currentAA);
                }

                const char* lightingModels[] = { "PBR Filmic (Cook-Torrance GGX)", "Advanced Blinn-Phong", "Soft Irradiance" };
                int currentLighting = static_cast<int>(rs.Lighting);
                if (ImGui::Combo("Lighting Model", &currentLighting, lightingModels, IM_ARRAYSIZE(lightingModels))) {
                    rs.Lighting = static_cast<LightingModel>(currentLighting);
                }

                const char* toneModes[] = { "ACES Filmic (Unreal Engine Standard)", "Reinhard", "Neutral", "AgX" };
                int currentTone = static_cast<int>(rs.ToneMapping);
                if (ImGui::Combo("Tone Mapping", &currentTone, toneModes, IM_ARRAYSIZE(toneModes))) {
                    rs.ToneMapping = static_cast<ToneMappingMode>(currentTone);
                }

                ImGui::SliderFloat("Resolution Scale", &rs.ResolutionScale, 0.5f, 2.0f, "%.2fx");
            }

            if (ImGui::CollapsingHeader("Lighting, Shadows & Post-Processing", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Dynamic Shadows", &rs.bEnableShadows);
                if (rs.bEnableShadows) {
                    const char* shadowRes[] = { "1024x1024 (Performance)", "2048x2048 (High)", "4096x4096 (Ultra)" };
                    int currentRes = (rs.ShadowResolution >= 4096) ? 2 : ((rs.ShadowResolution >= 2048) ? 1 : 0);
                    if (ImGui::Combo("Shadow Map Res", &currentRes, shadowRes, IM_ARRAYSIZE(shadowRes))) {
                        rs.ShadowResolution = (currentRes == 2) ? 4096 : ((currentRes == 1) ? 2048 : 1024);
                    }
                }

                ImGui::Checkbox("Ambient Occlusion (SSAO)", &rs.bEnableAmbientOcclusion);
                if (rs.bEnableAmbientOcclusion) {
                    ImGui::SliderFloat("AO Intensity", &rs.AmbientOcclusionStrength, 0.1f, 1.0f);
                }

                ImGui::Checkbox("HDR Bloom", &rs.bEnableBloom);
                if (rs.bEnableBloom) {
                    ImGui::SliderFloat("Bloom Intensity", &rs.BloomIntensity, 0.05f, 1.5f);
                    ImGui::SliderFloat("Bloom Threshold", &rs.BloomThreshold, 0.5f, 1.5f);
                }

                ImGui::SliderFloat("Camera Exposure", &rs.Exposure, 0.2f, 3.0f);
                ImGui::SliderFloat("Display Gamma", &rs.Gamma, 1.0f, 3.0f);
                ImGui::Checkbox("Custom Shader Volumes Active", &rs.bPostProcessShadersEnabled);
            }

            if (ImGui::CollapsingHeader("Editor & Viewport Preferences", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("Camera Speed", &m_CameraBaseSpeed, 1.0f, 20.0f);
                ImGui::SliderFloat("Mouse Sensitivity", &m_MouseSensitivity, 0.05f, 1.0f);
                ImGui::Checkbox("VSync Enabled", &m_bVSync);
            }

            ImGui::Separator();
            ImGui::TextDisabled("Active Graphics Backend: Vulkan 1.4 Dynamic Rendering");
            ImGui::TextDisabled("PBR Microfacet: Cook-Torrance GGX + ACES Filmic Curve");

            ImGui::Spacing();
            if (ImGui::Button("OK", ImVec2(100, 26))) {
                m_bShowEngineSettings = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

} // namespace Engine
