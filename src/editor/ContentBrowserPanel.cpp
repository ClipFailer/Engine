/**
 * @file ContentBrowserPanel.cpp
 * @brief Implementation of ContentBrowserPanel.
 */

#include "ContentBrowserPanel.hpp"
#include "scene/Scene.hpp"
#include "core/Log.hpp"
#include <imgui.h>

namespace Engine {

    ContentBrowserPanel::ContentBrowserPanel(Project& project)
        : m_Project(project) {
        m_RootPath = project.GetProjectPath().parent_path();
        if (m_RootPath.empty() || !std::filesystem::exists(m_RootPath)) {
            m_RootPath = std::filesystem::current_path();
        }
        m_CurrentPath = m_RootPath;
    }

    void ContentBrowserPanel::Render(Scene& scene) {
        ImGui::Begin("Content Browser");

        DrawBreadcrumbs();
        ImGui::Separator();

        // Split view: Left tree, Right files
        ImGui::Columns(2, "ContentBrowserSplit", true);
        ImGui::SetColumnWidth(0, 180.0f);

        // Left directory tree
        ImGui::BeginChild("FolderTree", ImVec2(0, 0), false);
        DrawDirectoryTree(m_RootPath);
        ImGui::EndChild();

        ImGui::NextColumn();

        // Right file grid
        ImGui::BeginChild("FileGrid", ImVec2(0, 0), false);
        DrawFileGrid(scene);
        ImGui::EndChild();

        ImGui::Columns(1);

        // Modal for Creating New C++ Class
        if (m_bShowCreateClassModal) {
            ImGui::OpenPopup("Create C++ Actor Class");
        }

        if (ImGui::BeginPopupModal("Create C++ Actor Class", &m_bShowCreateClassModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter new Actor class name (derived from Engine::Actor):");
            ImGui::InputText("##ClassName", m_NewClassName, sizeof(m_NewClassName));
            ImGui::Spacing();

            if (ImGui::Button("Create Class", ImVec2(120, 0))) {
                if (m_Project.GenerateActorClass(m_NewClassName)) {
                    LOG_INFO("Created C++ Actor class: {}", m_NewClassName);
                }
                m_bShowCreateClassModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                m_bShowCreateClassModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void ContentBrowserPanel::DrawBreadcrumbs() {
        if (m_CurrentPath != m_RootPath) {
            if (ImGui::Button("<- Back")) {
                m_CurrentPath = m_CurrentPath.parent_path();
            }
            ImGui::SameLine();
        }

        std::string relPath = std::filesystem::relative(m_CurrentPath, m_RootPath).string();
        if (relPath == ".") relPath = "Root";
        ImGui::TextDisabled("Path: %s", relPath.c_str());
    }

    void ContentBrowserPanel::DrawDirectoryTree(const std::filesystem::path& dir) {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.is_directory()) {
                    std::string dirName = entry.path().filename().string();
                    if (dirName.starts_with(".") || dirName == "build" || dirName == "external") continue;

                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (m_CurrentPath == entry.path()) flags |= ImGuiTreeNodeFlags_Selected;

                    bool opened = ImGui::TreeNodeEx(dirName.c_str(), flags, "%s", dirName.c_str());
                    if (ImGui::IsItemClicked()) {
                        m_CurrentPath = entry.path();
                    }

                    if (opened) {
                        DrawDirectoryTree(entry.path());
                        ImGui::TreePop();
                    }
                }
            }
        } catch (...) {}
    }

    void ContentBrowserPanel::DrawFileGrid(Scene& scene) {
        // Right click menu
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("New C++ Actor Class...")) {
                m_bShowCreateClassModal = true;
            }
            if (ImGui::MenuItem("New Folder")) {
                std::filesystem::create_directory(m_CurrentPath / "NewFolder");
            }
            if (ImGui::MenuItem("Save Active Scene Here")) {
                scene.SaveToFile((m_CurrentPath / "Scene.json").string());
            }
            ImGui::EndPopup();
        }

        float panelWidth = ImGui::GetContentRegionAvail().x;
        float cellSize = 90.0f;
        int columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));
        ImGui::Columns(columnCount, nullptr, false);

        try {
            for (const auto& entry : std::filesystem::directory_iterator(m_CurrentPath)) {
                std::string filename = entry.path().filename().string();
                if (filename.starts_with(".")) continue;

                ImGui::PushID(filename.c_str());

                bool isDir = entry.is_directory();
                ImVec4 btnColor = isDir ? ImVec4(0.25f, 0.35f, 0.45f, 1.0f) : ImVec4(0.20f, 0.20f, 0.23f, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
                if (ImGui::Button(isDir ? "[DIR]" : "[FILE]", ImVec2(70.0f, 50.0f))) {
                    if (isDir) {
                        m_CurrentPath /= entry.path().filename();
                        ImGui::PopStyleColor();
                        ImGui::PopID();
                        break;
                    } else if (entry.path().extension() == ".json") {
                        scene.LoadFromFile(entry.path().string());
                    }
                }
                ImGui::PopStyleColor();

                ImGui::TextWrapped("%s", filename.c_str());
                ImGui::NextColumn();

                ImGui::PopID();
            }
        } catch (...) {}

        ImGui::Columns(1);
    }

} // namespace Engine
