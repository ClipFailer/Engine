/**
 * @file ConsolePanel.cpp
 * @brief Implementation of ConsolePanel.
 */

#include "ConsolePanel.hpp"
#include <imgui.h>

namespace Engine {

    void ConsolePanel::Render() {
        ImGui::Begin("Output Log");

        // Top toolbar
        if (ImGui::Button("Clear")) {
            Logger::Clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_bAutoScroll);
        ImGui::SameLine();
        ImGui::Checkbox("Info", &m_bShowInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warnings", &m_bShowWarnings);
        ImGui::SameLine();
        ImGui::Checkbox("Errors", &m_bShowErrors);
        ImGui::SameLine();
        ImGui::InputTextWithHint("##LogFilter", "Filter Log...", m_FilterBuffer, sizeof(m_FilterBuffer));

        ImGui::Separator();

        // Log messages list
        ImGui::BeginChild("LogScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        std::string filterStr(m_FilterBuffer);
        auto messages = Logger::GetMessages();

        for (const auto& msg : messages) {
            if (msg.Level == LogLevel::Info && !m_bShowInfo) continue;
            if (msg.Level == LogLevel::Warning && !m_bShowWarnings) continue;
            if (msg.Level == LogLevel::Error && !m_bShowErrors) continue;

            if (!filterStr.empty() && msg.Message.find(filterStr) == std::string::npos) {
                continue;
            }

            ImVec4 color;
            switch (msg.Level) {
                case LogLevel::Info:    color = ImVec4(0.85f, 0.85f, 0.88f, 1.0f); break;
                case LogLevel::Warning: color = ImVec4(0.95f, 0.75f, 0.20f, 1.0f); break;
                case LogLevel::Error:   color = ImVec4(0.95f, 0.25f, 0.25f, 1.0f); break;
            }

            ImGui::TextDisabled("[%s]", msg.Time.c_str());
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", msg.Message.c_str());
        }

        if (m_bAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
    }

} // namespace Engine
