/**
 * @file ContentBrowserPanel.cpp
 * @brief Implementation of ContentBrowserPanel with file opening, spawning actors, and asset management.
 */

#include "ContentBrowserPanel.hpp"
#include "scene/Scene.hpp"
#include "core/Log.hpp"
#include <imgui.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#endif

namespace Engine {

    static void OpenInExplorer(const std::filesystem::path& path) {
#if defined(_WIN32)
        std::string p = path.string();
        ShellExecuteA(NULL, "explore", p.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
        std::string cmd = "xdg-open \"" + path.string() + "\" &";
        system(cmd.c_str());
#endif
    }

    ContentBrowserPanel::ContentBrowserPanel(Project& project) {
        SetProject(project);
    }

    void ContentBrowserPanel::SetProject(Project& project) {
        m_Project = &project;
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
        ImGui::SetColumnWidth(0, 190.0f);

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

        // Modal: Create C++ Actor Class
        if (m_bShowCreateClassModal) {
            ImGui::OpenPopup("Create C++ Actor Class###CreateActorModal");
        }
        if (ImGui::BeginPopupModal("Create C++ Actor Class###CreateActorModal", &m_bShowCreateClassModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter new Actor class name (derived from Engine::Actor):");
            ImGui::InputText("##ActorClassName", m_NewClassName, sizeof(m_NewClassName));
            ImGui::Spacing();

            if (ImGui::Button("Create Actor Class", ImVec2(140, 0))) {
                if (m_Project && m_Project->GenerateActorClass(m_NewClassName)) {
                    LOG_INFO("Created C++ Actor class: {}", m_NewClassName);
                }
                m_bShowCreateClassModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                m_bShowCreateClassModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Modal: Create C++ Component Class
        if (m_bShowCreateComponentModal) {
            ImGui::OpenPopup("Create C++ Component Class###CreateCompModal");
        }
        if (ImGui::BeginPopupModal("Create C++ Component Class###CreateCompModal", &m_bShowCreateComponentModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter new Component class name (derived from Engine::Component):");
            ImGui::InputText("##CompClassName", m_NewComponentName, sizeof(m_NewComponentName));
            ImGui::Spacing();

            if (ImGui::Button("Create Component Class", ImVec2(160, 0))) {
                if (m_Project && m_Project->GenerateComponentClass(m_NewComponentName)) {
                    LOG_INFO("Created C++ Component class: {}", m_NewComponentName);
                }
                m_bShowCreateComponentModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                m_bShowCreateComponentModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Modal: Rename Item
        if (m_bShowRenameModal) {
            ImGui::OpenPopup("Rename Item###RenameModal");
        }
        if (ImGui::BeginPopupModal("Rename Item###RenameModal", &m_bShowRenameModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("New name for '%s':", m_ItemToRename.filename().string().c_str());
            ImGui::InputText("##NewItemName", m_RenameBuffer, sizeof(m_RenameBuffer));
            ImGui::Spacing();

            if (ImGui::Button("Rename", ImVec2(100, 0))) {
                if (strlen(m_RenameBuffer) > 0) {
                    std::filesystem::path newPath = m_ItemToRename.parent_path() / m_RenameBuffer;
                    std::error_code ec;
                    std::filesystem::rename(m_ItemToRename, newPath, ec);
                    if (ec) {
                        LOG_ERROR("Failed to rename {}: {}", m_ItemToRename.string(), ec.message());
                    } else {
                        LOG_INFO("Renamed {} -> {}", m_ItemToRename.filename().string(), m_RenameBuffer);
                    }
                }
                m_bShowRenameModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                m_bShowRenameModal = false;
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

            // Drag drop target to move files up one level
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                    std::filesystem::path srcPath = (const char*)payload->Data;
                    std::filesystem::path dstPath = m_CurrentPath.parent_path() / srcPath.filename();
                    std::error_code ec;
                    std::filesystem::rename(srcPath, dstPath, ec);
                    if (!ec) {
                        LOG_INFO("Moved {} to parent folder", srcPath.filename().string());
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
        }

        std::string relPath = std::filesystem::relative(m_CurrentPath, m_RootPath).string();
        if (relPath == ".") relPath = "Root";
        ImGui::TextDisabled("Folder: %s", relPath.c_str());
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

                    // Drag and drop target onto folder node
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                            std::filesystem::path srcPath = (const char*)payload->Data;
                            std::filesystem::path dstPath = entry.path() / srcPath.filename();
                            std::error_code ec;
                            std::filesystem::rename(srcPath, dstPath, ec);
                            if (!ec) {
                                LOG_INFO("Moved {} into {}", srcPath.filename().string(), dirName);
                            }
                        }
                        ImGui::EndDragDropTarget();
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
        // Background right-click context menu
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("New C++ Actor Class...")) {
                m_bShowCreateClassModal = true;
            }
            if (ImGui::MenuItem("New C++ Component Class...")) {
                m_bShowCreateComponentModal = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("New Folder")) {
                std::filesystem::create_directory(m_CurrentPath / "NewFolder");
            }
            if (ImGui::MenuItem("Save Active Scene Here")) {
                scene.SaveToFile((m_CurrentPath / "Scene.json").string());
            }
            if (ImGui::MenuItem("Show in File Explorer")) {
                OpenInExplorer(m_CurrentPath);
            }
            ImGui::EndPopup();
        }

        float panelWidth = ImGui::GetContentRegionAvail().x;
        float cellSize = 95.0f;
        int columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));
        ImGui::Columns(columnCount, nullptr, false);

        std::filesystem::path itemToDelete;
        bool bDoDelete = false;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(m_CurrentPath)) {
                std::string filename = entry.path().filename().string();
                if (filename.starts_with(".")) continue;

                ImGui::PushID(filename.c_str());

                bool isDir = entry.is_directory();
                std::string ext = entry.path().extension().string();
                bool isCppOrHeader = (ext == ".cpp" || ext == ".hpp" || ext == ".h");

                ImVec4 btnColor = isDir ? ImVec4(0.22f, 0.35f, 0.48f, 1.0f) :
                                  isCppOrHeader ? ImVec4(0.25f, 0.40f, 0.25f, 1.0f) :
                                  ImVec4(0.18f, 0.18f, 0.22f, 1.0f);

                const char* iconLabel = isDir ? "[DIR]" :
                                        (ext == ".cpp") ? "[CPP]" :
                                        (ext == ".hpp" || ext == ".h") ? "[HPP]" :
                                        (ext == ".json") ? "[SCENE]" : "[FILE]";

                ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
                if (ImGui::Button(iconLabel, ImVec2(76.0f, 52.0f))) {
                    if (isDir) {
                        m_CurrentPath /= entry.path().filename();
                        ImGui::PopStyleColor();
                        ImGui::PopID();
                        break;
                    } else if (ext == ".json") {
                        scene.LoadFromFile(entry.path().string());
                    }
                }
                ImGui::PopStyleColor();

                // Double-click to open file / enter directory
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (isDir) {
                        m_CurrentPath /= entry.path().filename();
                        ImGui::PopID();
                        break;
                    } else {
                        if (OnFileDoubleClicked) {
                            OnFileDoubleClicked(entry.path());
                        }
                    }
                }

                // Drag source: drag file or folder to move it
                if (ImGui::BeginDragDropSource()) {
                    std::string pathStr = entry.path().string();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", pathStr.c_str(), pathStr.size() + 1);
                    ImGui::Text("Move %s", filename.c_str());
                    ImGui::EndDragDropSource();
                }

                // Drag target: drop onto a folder to move item into it
                if (isDir && ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                        std::filesystem::path srcPath = (const char*)payload->Data;
                        std::filesystem::path dstPath = entry.path() / srcPath.filename();
                        std::error_code ec;
                        std::filesystem::rename(srcPath, dstPath, ec);
                        if (!ec) {
                            LOG_INFO("Moved {} into {}", srcPath.filename().string(), filename);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                // Item Right-Click Context Menu
                if (ImGui::BeginPopupContextItem()) {
                    if (isCppOrHeader) {
                        std::string stemName = entry.path().stem().string();
                        std::string spawnLabel = "Spawn Object in Scene (" + stemName + ")";
                        if (ImGui::MenuItem(spawnLabel.c_str())) {
                            auto newActor = scene.SpawnActor(stemName);
                            newActor->Transform.Position = glm::vec3(0.0f, 1.5f, 0.0f);
                            if (OnSpawnActorFromClass) {
                                OnSpawnActorFromClass(stemName);
                            }
                            LOG_INFO("Created Actor '{}' on scene from code file {}", stemName, filename);
                        }
                        ImGui::Separator();
                    }

                    if (!isDir && ImGui::MenuItem("Open File (Editor & VS Code)")) {
                        if (OnFileDoubleClicked) {
                            OnFileDoubleClicked(entry.path());
                        }
                    }

                    if (ImGui::MenuItem("Rename...")) {
                        m_ItemToRename = entry.path();
                        std::strncpy(m_RenameBuffer, filename.c_str(), sizeof(m_RenameBuffer) - 1);
                        m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                        m_bShowRenameModal = true;
                    }

                    if (ImGui::MenuItem("Delete")) {
                        itemToDelete = entry.path();
                        bDoDelete = true;
                    }

                    ImGui::Separator();
                    if (ImGui::MenuItem("Show in File Explorer")) {
                        OpenInExplorer(isDir ? entry.path() : entry.path().parent_path());
                    }

                    ImGui::EndPopup();
                }

                ImGui::TextWrapped("%s", filename.c_str());
                ImGui::NextColumn();

                ImGui::PopID();
            }
        } catch (...) {}

        if (bDoDelete && !itemToDelete.empty()) {
            std::error_code ec;
            std::filesystem::remove_all(itemToDelete, ec);
            if (!ec) {
                LOG_INFO("Deleted file: {}", itemToDelete.filename().string());
            } else {
                LOG_ERROR("Failed to delete {}: {}", itemToDelete.string(), ec.message());
            }
        }

        ImGui::Columns(1);
    }

} // namespace Engine
