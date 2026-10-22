/**
 * @file UIDesignerWindow.cpp
 * @brief Implementation of UIDesignerWindow live visual canvas and HTML+CSS authoring.
 */

#include "UIDesignerWindow.hpp"
#include "scene/Actor.hpp"
#include "scene/WidgetComponent.hpp"
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

    UIDesignerWindow::UIDesignerWindow() {
        m_HTMLSource = "<div class='panel'>\n  <h1>Game HUD</h1>\n  <p>HP: 100/100</p>\n  <button>Action</button>\n</div>";
        m_CSSSource = ".panel {\n  background: #1b1e24;\n  color: #38d9a9;\n  padding: 12px;\n  border-radius: 6px;\n}";

        m_HTMLBuffer.resize(16384);
        std::memcpy(m_HTMLBuffer.data(), m_HTMLSource.c_str(), m_HTMLSource.size() + 1);

        m_CSSBuffer.resize(16384);
        std::memcpy(m_CSSBuffer.data(), m_CSSSource.c_str(), m_CSSSource.size() + 1);

        m_bNeedsRebuild = true;
    }

    void UIDesignerWindow::OpenFile(const std::filesystem::path& filePath) {
        m_ActiveFilePath = filePath;
        std::ifstream file(filePath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();

            // Look for <style> separator if single file
            size_t stylePos = content.find("<style>");
            if (stylePos != std::string::npos) {
                m_HTMLSource = content.substr(0, stylePos);
                size_t styleEnd = content.find("</style>", stylePos);
                if (styleEnd != std::string::npos) {
                    m_CSSSource = content.substr(stylePos + 7, styleEnd - (stylePos + 7));
                } else {
                    m_CSSSource = content.substr(stylePos + 7);
                }
            } else {
                m_HTMLSource = content;
                m_CSSSource = ".hud { background: #222; color: #fff; padding: 10px; }";
            }

            m_HTMLBuffer.resize(std::max<size_t>(m_HTMLSource.size() + 4096, 16384));
            std::memcpy(m_HTMLBuffer.data(), m_HTMLSource.c_str(), m_HTMLSource.size() + 1);

            m_CSSBuffer.resize(std::max<size_t>(m_CSSSource.size() + 4096, 16384));
            std::memcpy(m_CSSBuffer.data(), m_CSSSource.c_str(), m_CSSSource.size() + 1);

            m_bNeedsRebuild = true;
            m_bIsOpen = true;
            LOG_INFO("UI Designer opened: {}", filePath.string());
        }
    }

    void UIDesignerWindow::Render(Actor* selectedActor) {
        if (!m_bIsOpen) return;

        ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
        std::string title = "UI Designer & Live Preview - " +
            (m_ActiveFilePath.empty() ? "New Widget" : m_ActiveFilePath.filename().string()) + "###UIDesignerWindow";

        if (ImGui::Begin(title.c_str(), &m_bIsOpen, ImGuiWindowFlags_MenuBar)) {
            // Menu bar
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Save", "Ctrl+S")) {
                        if (!m_ActiveFilePath.empty()) {
                            std::ofstream out(m_ActiveFilePath);
                            if (out.is_open()) {
                                out << m_HTMLBuffer.data() << "\n<style>\n" << m_CSSBuffer.data() << "\n</style>\n";
                                LOG_INFO("Saved UI Widget: {}", m_ActiveFilePath.string());
                            }
                        }
                    }
                    if (ImGui::MenuItem("Open in VS Code")) {
                        if (!m_ActiveFilePath.empty()) {
#if defined(_WIN32)
                            ShellExecuteA(NULL, "open", m_ActiveFilePath.string().c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
                            std::string cmd = "code \"" + m_ActiveFilePath.string() + "\" &";
                            system(cmd.c_str());
#endif
                        }
                    }
                    if (ImGui::MenuItem("Close")) m_bIsOpen = false;
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Templates")) {
                    if (ImGui::MenuItem("Health Bar Template")) {
                        std::string h = "<div class='hp-bar'>\n  <h1>Player HP</h1>\n  <p>100 / 100</p>\n</div>";
                        std::string c = ".hp-bar { background: #c92a2a; color: #fff; padding: 8px; border-radius: 4px; }";
                        std::memcpy(m_HTMLBuffer.data(), h.c_str(), h.size() + 1);
                        std::memcpy(m_CSSBuffer.data(), c.c_str(), c.size() + 1);
                        m_bNeedsRebuild = true;
                    }
                    if (ImGui::MenuItem("Button Card Template")) {
                        std::string h = "<div class='card'>\n  <h1>Interact</h1>\n  <button>Press E to Open</button>\n</div>";
                        std::string c = ".card { background: #1c7ed6; color: #fff; padding: 12px; border-radius: 8px; }";
                        std::memcpy(m_HTMLBuffer.data(), h.c_str(), h.size() + 1);
                        std::memcpy(m_CSSBuffer.data(), c.c_str(), c.size() + 1);
                        m_bNeedsRebuild = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            // Top Toolbar
            if (ImGui::Button("Save Widget")) {
                if (!m_ActiveFilePath.empty()) {
                    std::ofstream out(m_ActiveFilePath);
                    if (out.is_open()) {
                        out << m_HTMLBuffer.data() << "\n<style>\n" << m_CSSBuffer.data() << "\n</style>\n";
                        LOG_INFO("Saved UI Widget: {}", m_ActiveFilePath.string());
                    }
                }
            }
            if (!m_ActiveFilePath.empty()) {
                ImGui::SameLine();
                if (ImGui::Button("Open in VS Code")) {
#if defined(_WIN32)
                    ShellExecuteA(NULL, "open", m_ActiveFilePath.string().c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
                    std::string cmd = "code \"" + m_ActiveFilePath.string() + "\" &";
                    system(cmd.c_str());
#endif
                }
            }
            ImGui::SameLine();
            if (selectedActor) {
                if (ImGui::Button("Attach to Selected Actor")) {
                    auto widgetComp = selectedActor->AddComponent<WidgetComponent>(
                        std::string(m_HTMLBuffer.data()),
                        std::string(m_CSSBuffer.data())
                    );
                    widgetComp->bIsWorldSpace = true;
                    LOG_INFO("Attached WidgetComponent to Actor '{}'", selectedActor->GetName());
                }
                ImGui::SameLine();
                ImGui::TextDisabled("Target: %s", selectedActor->GetName().c_str());
            }

            ImGui::Separator();

            // Rebuild document DOM if modified
            if (m_bNeedsRebuild) {
                m_Document.ParseHTML(m_HTMLBuffer.data());
                m_Document.ParseCSS(m_CSSBuffer.data());
                m_bNeedsRebuild = false;
            }

            // Two-column layout: Left = Code Editor, Right = Live Visual Preview
            ImGui::Columns(2, "UIDesignerSplit", true);
            ImGui::SetColumnWidth(0, 420.0f);

            // Left Column: HTML & CSS Code
            ImGui::BeginChild("CodeTabs", ImVec2(0, 0), false);
            if (ImGui::BeginTabBar("UIDesignerTabs")) {
                if (ImGui::BeginTabItem("HTML Structure")) {
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    if (ImGui::InputTextMultiline("##HTMLSource", m_HTMLBuffer.data(), m_HTMLBuffer.size(), avail, ImGuiInputTextFlags_AllowTabInput)) {
                        m_bNeedsRebuild = true;
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("CSS Styling")) {
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    if (ImGui::InputTextMultiline("##CSSSource", m_CSSBuffer.data(), m_CSSBuffer.size(), avail, ImGuiInputTextFlags_AllowTabInput)) {
                        m_bNeedsRebuild = true;
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();

            ImGui::NextColumn();

            // Right Column: Live Interactive Visual Canvas
            ImGui::BeginChild("LivePreviewCanvas", ImVec2(0, 0), true);
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Live Visual Canvas (HTML+CSS Engine)");
            ImGui::Separator();

            ImVec2 previewPos = ImGui::GetCursorScreenPos();
            ImVec2 previewAvail = ImGui::GetContentRegionAvail();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // Checkerboard / Slate background for preview
            drawList->AddRectFilled(
                previewPos,
                ImVec2(previewPos.x + previewAvail.x, previewPos.y + previewAvail.y),
                IM_COL32(20, 22, 26, 255)
            );

            // Render live UI DOM document
            m_Document.Render(drawList, ImVec2(previewPos.x + 20.0f, previewPos.y + 20.0f), ImVec2(previewAvail.x - 40.0f, previewAvail.y - 40.0f));

            ImGui::EndChild();
            ImGui::Columns(1);
        }
        ImGui::End();
    }

} // namespace Engine
