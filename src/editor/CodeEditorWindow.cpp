/**
 * @file CodeEditorWindow.cpp
 * @brief Implementation of integrated in-engine code editor.
 */

#include "CodeEditorWindow.hpp"
#include "core/Log.hpp"
#include <imgui.h>
#include <fstream>
#include <sstream>

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

    static void LaunchExternalEditor(const std::filesystem::path& path) {
#if defined(_WIN32)
        ShellExecuteA(NULL, "open", path.string().c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
        std::string cmd = "code \"" + path.string() + "\" || xdg-open \"" + path.string() + "\" &";
        system(cmd.c_str());
#endif
    }

    void CodeEditorWindow::OpenFile(const std::filesystem::path& filePath) {
        if (!std::filesystem::exists(filePath)) {
            LOG_ERROR("CodeEditor: File does not exist: {}", filePath.string());
            return;
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("CodeEditor: Unable to open file: {}", filePath.string());
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        m_FileContent = buffer.str();
        m_ActiveFilePath = filePath;

        // Allocate buffer with headroom for edits
        m_TextBuffer.resize(std::max<size_t>(m_FileContent.size() + 4096, 65536));
        std::memcpy(m_TextBuffer.data(), m_FileContent.c_str(), m_FileContent.size());
        m_TextBuffer[m_FileContent.size()] = '\0';

        m_bIsModified = false;
        m_bIsOpen = true;
        m_StatusMessage = "Loaded: " + filePath.filename().string();
        LOG_INFO("CodeEditor opened: {}", filePath.string());

        // Also launch in VS Code / system editor for developer convenience
        LaunchExternalEditor(filePath);
    }

    void CodeEditorWindow::Render() {
        if (!m_bIsOpen) return;

        ImGui::SetNextWindowSize(ImVec2(750, 550), ImGuiCond_FirstUseEver);
        std::string title = "Source Editor - " + (m_ActiveFilePath.empty() ? "No File" : m_ActiveFilePath.filename().string());
        if (m_bIsModified) title += " *";
        title += "###CodeEditorWindow";

        if (ImGui::Begin(title.c_str(), &m_bIsOpen, ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Save", "Ctrl+S")) {
                        std::ofstream out(m_ActiveFilePath);
                        if (out.is_open()) {
                            out << m_TextBuffer.data();
                            out.close();
                            m_bIsModified = false;
                            m_StatusMessage = "Saved successfully!";
                            LOG_INFO("CodeEditor saved file: {}", m_ActiveFilePath.string());
                        }
                    }
                    if (ImGui::MenuItem("Open in VS Code / System Editor")) {
                        LaunchExternalEditor(m_ActiveFilePath);
                    }
                    if (ImGui::MenuItem("Close")) {
                        m_bIsOpen = false;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            // Toolbar
            if (ImGui::Button("Save Changes")) {
                std::ofstream out(m_ActiveFilePath);
                if (out.is_open()) {
                    out << m_TextBuffer.data();
                    out.close();
                    m_bIsModified = false;
                    m_StatusMessage = "File saved successfully!";
                    LOG_INFO("CodeEditor saved file: {}", m_ActiveFilePath.string());
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Open in VS Code")) {
                LaunchExternalEditor(m_ActiveFilePath);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("File: %s", m_ActiveFilePath.string().c_str());

            ImGui::Separator();

            // Text editing area with monospace font style
            ImVec2 avail = ImGui::GetContentRegionAvail();
            avail.y -= 26.0f; // Leave space for status bar

            ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
            if (ImGui::InputTextMultiline("##SourceCode", m_TextBuffer.data(), m_TextBuffer.size(), avail, flags)) {
                m_bIsModified = true;
            }

            // Status bar
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "%s", m_StatusMessage.c_str());
            ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f);
            ImGui::TextDisabled("Length: %zu chars", std::strlen(m_TextBuffer.data()));
        }
        ImGui::End();
    }

} // namespace Engine
