/**
 * @file ProjectHubWindow.cpp
 * @brief Implementation of Startup Project Hub launcher.
 */

#include "ProjectHubWindow.hpp"
#include "core/Log.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#endif

namespace Engine {

    using json = nlohmann::json;

    ProjectHubWindow::ProjectHubWindow() {
        LoadRecentProjects();
    }

    void ProjectHubWindow::LoadRecentProjects() {
        m_RecentProjects.clear();
        std::filesystem::path configPath = "recent_projects.json";
        if (!std::filesystem::exists(configPath)) {
            // Add default fallback if exists
            if (std::filesystem::exists("DefaultProject/DefaultProject.uproj")) {
                m_RecentProjects.push_back({"DefaultProject", "DefaultProject/DefaultProject.uproj", "Current"});
            }
            return;
        }

        try {
            std::ifstream file(configPath);
            json j;
            file >> j;
            for (const auto& item : j) {
                m_RecentProjects.push_back({
                    item.value("name", "Project"),
                    item.value("path", ""),
                    item.value("date", "")
                });
            }
        } catch (...) {}
    }

    void ProjectHubWindow::SaveRecentProjects() {
        try {
            json j = json::array();
            for (const auto& p : m_RecentProjects) {
                j.push_back({
                    {"name", p.Name},
                    {"path", p.Path},
                    {"date", p.LastOpened}
                });
            }
            std::ofstream file("recent_projects.json");
            file << j.dump(4);
        } catch (...) {}
    }

    void ProjectHubWindow::AddRecentProject(const std::string& name, const std::string& path) {
        // Remove duplicate
        for (auto it = m_RecentProjects.begin(); it != m_RecentProjects.end(); ++it) {
            if (it->Path == path) {
                m_RecentProjects.erase(it);
                break;
            }
        }

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M");

        m_RecentProjects.insert(m_RecentProjects.begin(), {name, path, ss.str()});
        if (m_RecentProjects.size() > 10) m_RecentProjects.resize(10);
        SaveRecentProjects();
    }

    void ProjectHubWindow::Render() {
        if (!m_bIsOpen) return;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImVec2 center = vp->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(780, 480), ImGuiCond_FirstUseEver);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;

        if (ImGui::Begin("Engine Project Hub###ProjectHubWindow", &m_bIsOpen, flags)) {
            // Header
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
            ImGui::SetWindowFontScale(1.3f);
            ImGui::Text("VULKAN 3D ENGINE");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            ImGui::TextDisabled("Select an existing project or create a new gameplay simulation");
            ImGui::Separator();

            ImGui::Columns(2, "HubSplit", true);
            ImGui::SetColumnWidth(0, 180.0f);

            // Left Sidebar tabs
            if (ImGui::Selectable("Recent Projects", m_SelectedTab == 0, 0, ImVec2(0, 32.0f))) {
                m_SelectedTab = 0;
            }
            if (ImGui::Selectable("Create New Project", m_SelectedTab == 1, 0, ImVec2(0, 32.0f))) {
                m_SelectedTab = 1;
            }

            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Button("Browse Project (.uproj)", ImVec2(-1, 30.0f))) {
#if defined(_WIN32)
                char szFile[MAX_PATH] = "";
                OPENFILENAMEA ofn{};
                ofn.lStructSize = sizeof(ofn);
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "Engine Project (*.uproj)\0*.uproj\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileNameA(&ofn)) {
                    if (OnOpenProject) {
                        OnOpenProject(szFile);
                        m_bIsOpen = false;
                    }
                }
#endif
            }

            ImGui::NextColumn();

            // Right Content Area
            if (m_SelectedTab == 0) {
                // Recent Projects List
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Recently Opened Projects");
                ImGui::Separator();

                if (m_RecentProjects.empty()) {
                    ImGui::TextDisabled("No recent projects found. Click 'Create New Project' to start!");
                } else {
                    ImGui::BeginChild("RecentList", ImVec2(0, 0), false);
                    for (const auto& proj : m_RecentProjects) {
                        ImGui::PushID(proj.Path.c_str());

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.18f, 0.22f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.28f, 0.35f, 1.0f));

                        std::string label = proj.Name + " (" + proj.LastOpened + ")\n" + proj.Path;
                        if (ImGui::Button(label.c_str(), ImVec2(-1, 48.0f))) {
                            if (OnOpenProject) {
                                OnOpenProject(proj.Path);
                                m_bIsOpen = false;
                            }
                        }
                        ImGui::PopStyleColor(2);

                        ImGui::PopID();
                        ImGui::Spacing();
                    }
                    ImGui::EndChild();
                }
            } else {
                // New Project Wizard
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "New Project Settings");
                ImGui::Separator();

                ImGui::Text("Project Name:");
                if (ImGui::InputText("##NewProjName", m_NewProjectName, sizeof(m_NewProjectName))) {
                    std::string folder = "C:/Projects/Vulkan/" + std::string(m_NewProjectName);
                    std::strncpy(m_NewProjectPath, folder.c_str(), sizeof(m_NewProjectPath) - 1);
                }

                ImGui::Spacing();
                ImGui::Text("Location Directory:");
                ImGui::InputText("##NewProjPath", m_NewProjectPath, sizeof(m_NewProjectPath));

                ImGui::Spacing();
                ImGui::Checkbox("Initialize with Empty Scene (No starter actors)", &m_bEmptyScene);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.55f, 0.35f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.68f, 0.42f, 1.0f));
                if (ImGui::Button("Create Project", ImVec2(180, 36.0f))) {
                    if (OnCreateProject) {
                        OnCreateProject(m_NewProjectPath, m_NewProjectName, m_bEmptyScene);
                        m_bIsOpen = false;
                    }
                }
                ImGui::PopStyleColor(2);
            }

            ImGui::Columns(1);
        }
        ImGui::End();
    }

} // namespace Engine
