/**
 * @file ContentBrowserPanel.cpp
 * @brief Implementation of ContentBrowserPanel with class creation wizard, actor dragging, UI widget creation, and folder management.
 */

#include "ContentBrowserPanel.hpp"
#include "scene/Scene.hpp"
#include "reflection/Reflection.hpp"
#include "core/Log.hpp"
#include <imgui.h>
#include <fstream>
#include <algorithm>

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

    static void OpenInVSCode(const std::filesystem::path& path) {
#if defined(_WIN32)
        std::string p = path.string();
        ShellExecuteA(NULL, "open", p.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
        std::string cmd = "code \"" + path.string() + "\" &";
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

        // Modal: Create Class Wizard (with Parent Class Search)
        if (m_bShowCreateClassWizard) {
            ImGui::OpenPopup("Create C++ Class###CreateClassWizardModal");
        }
        if (ImGui::BeginPopupModal("Create C++ Class###CreateClassWizardModal", &m_bShowCreateClassWizard, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter New Class Name:");
            ImGui::InputText("##WizardClassName", m_WizardClassName, sizeof(m_WizardClassName));
            ImGui::Spacing();

            ImGui::Text("Select Parent Class:");
            ImGui::InputTextWithHint("##SearchParent", "Search parent class...", m_ParentSearchFilter, sizeof(m_ParentSearchFilter));

            std::vector<std::string> parentOptions = {"Actor", "Character", "Component", "UIWidget"};
            // Also add all reflected classes from TypeRegistry
            for (const auto& [name, desc] : TypeRegistry::Get().GetAllClasses()) {
                if (std::find(parentOptions.begin(), parentOptions.end(), name) == parentOptions.end()) {
                    parentOptions.push_back(name);
                }
            }

            std::string filter(m_ParentSearchFilter);
            std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

            ImGui::BeginChild("ParentClassList", ImVec2(340, 140), true);
            for (const auto& opt : parentOptions) {
                std::string lowerOpt = opt;
                std::transform(lowerOpt.begin(), lowerOpt.end(), lowerOpt.begin(), ::tolower);
                if (!filter.empty() && lowerOpt.find(filter) == std::string::npos) continue;

                bool isSelected = (std::string(m_WizardParentClass) == opt);
                if (ImGui::Selectable(opt.c_str(), isSelected)) {
                    std::strncpy(m_WizardParentClass, opt.c_str(), sizeof(m_WizardParentClass) - 1);
                }
            }
            ImGui::EndChild();

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Class %s will inherit from %s", m_WizardClassName, m_WizardParentClass);
            ImGui::Separator();

            if (ImGui::Button("Create Class", ImVec2(130, 28))) {
                std::string parentStr(m_WizardParentClass);
                std::string classStr(m_WizardClassName);

                if (parentStr == "UIWidget") {
                    // Create UI Widget file (.ui)
                    std::filesystem::path uiPath = m_CurrentPath / (classStr + ".ui");
                    std::ofstream out(uiPath);
                    out << "<div class='widget'>\n  <h1>" << classStr << "</h1>\n  <button>Click</button>\n</div>\n<style>\n.widget { background: #1c2026; color: #4dabf7; padding: 12px; border-radius: 6px; }\n</style>\n";
                    out.close();
                    LOG_INFO("Created UI Widget: {}", uiPath.string());
                } else if (parentStr == "Component") {
                    if (m_Project) m_Project->GenerateComponentClass(classStr);
                } else if (parentStr == "Character") {
                    // Generate Character subclass
                    std::filesystem::path srcDir = m_Project ? m_Project->GetSourceDirectory() : m_CurrentPath;
                    std::filesystem::create_directories(srcDir);
                    std::ofstream h(srcDir / (classStr + ".hpp"));
                    h << "#pragma once\n#include \"scene/Character.hpp\"\n\nclass " << classStr << " : public Engine::Character {\n    GENERATED_BODY()\npublic:\n    " << classStr << "();\n    void BeginPlay() override;\n};\n";
                    h.close();

                    std::ofstream cpp(srcDir / (classStr + ".cpp"));
                    cpp << "#include \"" << classStr << ".hpp\"\n\n" << classStr << "::" << classStr << "() : Engine::Character(\"" << classStr << "\") {}\nvoid " << classStr << "::BeginPlay() { Engine::Character::BeginPlay(); }\n\nENGINE_CLASS_BEGIN(" << classStr << ", Character)\nENGINE_CLASS_END()\n";
                    cpp.close();
                    LOG_INFO("Created Character C++ class: {}", classStr);
                } else {
                    if (m_Project) m_Project->GenerateActorClass(classStr);
                }

                m_bShowCreateClassWizard = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(90, 28))) {
                m_bShowCreateClassWizard = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Modal: Create Folder
        if (m_bShowNewFolderModal) {
            ImGui::OpenPopup("Create Folder###NewFolderModal");
        }
        if (ImGui::BeginPopupModal("Create Folder###NewFolderModal", &m_bShowNewFolderModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter Folder Name:");
            ImGui::InputText("##NewFolderName", m_NewFolderName, sizeof(m_NewFolderName));
            ImGui::Spacing();

            if (ImGui::Button("Create", ImVec2(100, 26))) {
                if (strlen(m_NewFolderName) > 0) {
                    std::filesystem::create_directories(m_CurrentPath / m_NewFolderName);
                    LOG_INFO("Created folder: {}", (m_CurrentPath / m_NewFolderName).string());
                }
                m_bShowNewFolderModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(80, 26))) {
                m_bShowNewFolderModal = false;
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

            if (ImGui::Button("Rename", ImVec2(100, 26))) {
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
            if (ImGui::Button("Cancel", ImVec2(80, 26))) {
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
            if (ImGui::MenuItem("Create Actor Blueprint (.actor)...")) {
                std::string baseName = "NewActor";
                std::filesystem::path newFile = m_CurrentPath / (baseName + ".actor");
                int counter = 1;
                while (std::filesystem::exists(newFile)) {
                    newFile = m_CurrentPath / (baseName + std::to_string(counter++) + ".actor");
                }
                Actor temp(newFile.stem().string());
                temp.SaveToAssetFile(newFile.string());
                LOG_INFO("Created new Actor Blueprint asset: {}", newFile.string());
            }
            if (ImGui::MenuItem("Create Class...")) {
                m_bShowCreateClassWizard = true;
            }
            if (ImGui::MenuItem("Create UI Widget...")) {
                std::filesystem::path uiPath = m_CurrentPath / "NewWidget.ui";
                std::ofstream out(uiPath);
                out << "<div class='widget'>\n  <h1>New UI Widget</h1>\n  <button>Interact</button>\n</div>\n<style>\n.widget { background: #1c2026; color: #4dabf7; padding: 12px; border-radius: 6px; }\n</style>\n";
                out.close();
                LOG_INFO("Created UI Widget: {}", uiPath.string());
            }
            ImGui::Separator();
            if (ImGui::MenuItem("New Folder...")) {
                m_bShowNewFolderModal = true;
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
        int columnCount = (std::max)(1, static_cast<int>(panelWidth / cellSize));
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
                bool isActorAsset = (ext == ".actor");
                bool isUIWidget = (ext == ".ui" || ext == ".uwidget");
                bool isScene = (ext == ".json" || ext == ".scene");

                ImVec4 btnColor = isDir ? ImVec4(0.22f, 0.35f, 0.48f, 1.0f) :
                                  isActorAsset ? ImVec4(0.2f, 0.45f, 0.8f, 1.0f) :
                                  isUIWidget ? ImVec4(0.18f, 0.52f, 0.50f, 1.0f) :
                                  isCppOrHeader ? ImVec4(0.25f, 0.40f, 0.25f, 1.0f) :
                                  isScene ? ImVec4(0.7f, 0.45f, 0.15f, 1.0f) :
                                  ImVec4(0.18f, 0.18f, 0.22f, 1.0f);

                const char* iconLabel = isDir ? "[DIR]" :
                                        isActorAsset ? "[ACTOR]" :
                                        isUIWidget ? "[UI]" :
                                        (ext == ".cpp") ? "[CPP]" :
                                        (ext == ".hpp" || ext == ".h") ? "[HPP]" :
                                        isScene ? "[SCENE]" : "[FILE]";

                ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
                if (ImGui::Button(iconLabel, ImVec2(76.0f, 52.0f))) {
                    if (isDir) {
                        m_CurrentPath /= entry.path().filename();
                        ImGui::PopStyleColor();
                        ImGui::PopID();
                        break;
                    }
                }
                ImGui::PopStyleColor();

                // Double-click actions
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (isDir) {
                        m_CurrentPath /= entry.path().filename();
                        ImGui::PopID();
                        break;
                    } else if (isActorAsset) {
                        if (OnActorAssetDoubleClicked) {
                            OnActorAssetDoubleClicked(entry.path().string());
                        }
                    } else if (isUIWidget) {
                        if (OnUIDoubleClicked) {
                            OnUIDoubleClicked(entry.path());
                        }
                    } else if (isScene) {
                        if (OnOpenScene) {
                            OnOpenScene(entry.path());
                        }
                    } else if (isCppOrHeader) {
                        // Double-click opens internally in CodeEditorWindow
                        if (OnFileDoubleClicked) {
                            OnFileDoubleClicked(entry.path());
                        }
                    }
                }

                // Drag source: drag onto 3D viewport or folders
                if (ImGui::BeginDragDropSource()) {
                    if (isActorAsset) {
                        std::string pathStr = entry.path().string();
                        ImGui::SetDragDropPayload("ACTOR_ASSET", pathStr.c_str(), pathStr.size() + 1);
                        ImGui::Text("Spawn Actor Asset: %s", filename.c_str());
                    } else if (isCppOrHeader) {
                        std::string stem = entry.path().stem().string();
                        ImGui::SetDragDropPayload("ACTOR_ASSET", stem.c_str(), stem.size() + 1);
                        ImGui::Text("Spawn C++ Actor: %s", stem.c_str());
                    } else {
                        std::string pathStr = entry.path().string();
                        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", pathStr.c_str(), pathStr.size() + 1);
                        ImGui::Text("Move %s", filename.c_str());
                    }
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
                    std::string stemName = entry.path().stem().string();

                    if (isCppOrHeader) {
                        // Create Actor Representation
                        std::string actorAssetLabel = "Create Actor Representation (" + stemName + ".actor)";
                        if (ImGui::MenuItem(actorAssetLabel.c_str())) {
                            std::filesystem::path actorFile = entry.path().parent_path() / (stemName + ".actor");
                            Actor temp(stemName);
                            temp.SaveToAssetFile(actorFile.string());
                            LOG_INFO("Created Actor Representation: {}", actorFile.string());
                        }

                        std::string spawnLabel = "Spawn in Scene (" + stemName + ")";
                        if (ImGui::MenuItem(spawnLabel.c_str())) {
                            auto newActor = scene.SpawnActor(stemName);
                            newActor->Transform.Position = glm::vec3(0.0f, 1.5f, 0.0f);
                            if (OnSpawnActorFromClass) {
                                OnSpawnActorFromClass(stemName);
                            }
                            LOG_INFO("Created Actor '{}' on scene from code file", stemName);
                        }

                        if (filename.find("Component") != std::string::npos) {
                            if (ImGui::MenuItem("Attach Component to Selected Actor")) {
                                if (OnAttachComponentToSelected) {
                                    OnAttachComponentToSelected(stemName);
                                }
                            }
                        }

                        ImGui::Separator();
                        if (ImGui::MenuItem("Open in VS Code")) {
                            OpenInVSCode(entry.path());
                        }
                        if (ImGui::MenuItem("Open in Internal Code Editor")) {
                            if (OnFileDoubleClicked) OnFileDoubleClicked(entry.path());
                        }
                    } else if (isActorAsset) {
                        if (ImGui::MenuItem("Spawn Actor in Scene")) {
                            if (OnSpawnActorFromAsset) {
                                OnSpawnActorFromAsset(entry.path().string());
                            } else {
                                scene.SpawnActorFromAsset(entry.path(), glm::vec3(0.0f, 1.5f, 0.0f));
                            }
                        }
                        if (ImGui::MenuItem("Open in 3D Actor Detail Window")) {
                            if (OnActorAssetDoubleClicked) OnActorAssetDoubleClicked(entry.path().string());
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Open in VS Code")) {
                            OpenInVSCode(entry.path());
                        }
                        if (ImGui::MenuItem("Open in Internal Code Editor")) {
                            if (OnFileDoubleClicked) OnFileDoubleClicked(entry.path());
                        }
                    } else if (isUIWidget) {
                        if (ImGui::MenuItem("Open in VS Code")) {
                            OpenInVSCode(entry.path());
                        }
                        if (ImGui::MenuItem("Open in UI Designer (Live Preview)")) {
                            if (OnUIDoubleClicked) OnUIDoubleClicked(entry.path());
                        }
                    } else if (isScene) {
                        if (ImGui::MenuItem("Open Scene in Editor")) {
                            if (OnOpenScene) OnOpenScene(entry.path());
                        }
                        if (ImGui::MenuItem("Open in Internal Code Editor")) {
                            if (OnFileDoubleClicked) OnFileDoubleClicked(entry.path());
                        }
                    } else if (!isDir) {
                        if (ImGui::MenuItem("Open in VS Code")) {
                            OpenInVSCode(entry.path());
                        }
                        if (ImGui::MenuItem("Open in Internal Code Editor")) {
                            if (OnFileDoubleClicked) OnFileDoubleClicked(entry.path());
                        }
                    }

                    ImGui::Separator();
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
