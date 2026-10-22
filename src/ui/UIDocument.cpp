/**
 * @file UIDocument.cpp
 * @brief Implementation of HTML+CSS parsing and rendering.
 */

#include "UIDocument.hpp"
#include <imgui.h>
#include <regex>
#include <stack>

namespace Engine {

    UIDocument::UIDocument() {
        m_Root = std::make_shared<UIElement>("root");
    }

    bool UIDocument::LoadFromString(const std::string& html) {
        m_Root = std::make_shared<UIElement>("root");
        m_Root->Style.Display = UIDisplay::Flex;
        m_Root->Style.FlexDirection = UIFlexDirection::Column;

        std::stack<std::shared_ptr<UIElement>> elementStack;
        elementStack.push(m_Root);

        // Simple HTML token regex: tags or text
        std::regex tokenRegex("<(/?[a-zA-Z0-9]+)([^>]*)>|([^<]+)");
        auto words_begin = std::sregex_iterator(html.begin(), html.end(), tokenRegex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;

            if (match[1].matched) { // Tag matched
                std::string tagName = match[1].str();
                std::string attrStr = match[2].str();

                if (tagName.starts_with("/")) {
                    // Closing tag
                    if (elementStack.size() > 1) {
                        elementStack.pop();
                    }
                } else {
                    // Opening tag
                    auto elem = std::make_shared<UIElement>(tagName);

                    // Parse attributes
                    std::regex attrRegex("([a-zA-Z0-9_-]+)=\"([^\"]*)\"");
                    auto ab = std::sregex_iterator(attrStr.begin(), attrStr.end(), attrRegex);
                    auto ae = std::sregex_iterator();
                    for (auto it = ab; it != ae; ++it) {
                        std::string k = (*it)[1].str();
                        std::string v = (*it)[2].str();
                        if (k == "id") elem->Id = v;
                        else if (k == "class") elem->ClassName = v;
                        else if (k == "style") elem->Style.ParseInline(v);
                    }

                    // Default tag styles
                    if (tagName == "button") {
                        if (elem->Style.BackgroundColor.a == 0.0f) elem->Style.BackgroundColor = glm::vec4(0.12f, 0.45f, 0.8f, 1.0f);
                        if (elem->Style.BorderRadius == 0.0f) elem->Style.BorderRadius = 4.0f;
                    }

                    elementStack.top()->AddChild(elem);
                    elementStack.push(elem);
                }
            } else if (match[3].matched) { // Text matched
                std::string text = match[3].str();
                text.erase(0, text.find_first_not_of(" \t\r\n"));
                text.erase(text.find_last_not_of(" \t\r\n") + 1);
                if (!text.empty() && !elementStack.empty()) {
                    elementStack.top()->Text += text;
                }
            }
        }

        return true;
    }

    void UIDocument::Render() {
        if (!m_Root) return;
        for (const auto& child : m_Root->Children) {
            RenderElement(child);
        }
    }

    void UIDocument::RenderElement(const std::shared_ptr<UIElement>& element) {
        if (!element) return;

        ImGui::PushID(element.get());

        // Apply CSS styles
        bool hasBg = (element->Style.BackgroundColor.a > 0.0f);
        if (hasBg) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(
                element->Style.BackgroundColor.r,
                element->Style.BackgroundColor.g,
                element->Style.BackgroundColor.b,
                element->Style.BackgroundColor.a
            ));
        }

        if (element->Tag == "button") {
            ImVec2 btnSize(element->Style.Width > 0 ? element->Style.Width : 0.0f,
                           element->Style.Height > 0 ? element->Style.Height : 0.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
                element->Style.BackgroundColor.r,
                element->Style.BackgroundColor.g,
                element->Style.BackgroundColor.b,
                element->Style.BackgroundColor.a
            ));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(
                element->Style.TextColor.r,
                element->Style.TextColor.g,
                element->Style.TextColor.b,
                element->Style.TextColor.a
            ));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, element->Style.BorderRadius);

            std::string label = element->Text.empty() ? "Button" : element->Text;
            if (ImGui::Button(label.c_str(), btnSize)) {
                if (element->OnClick) element->OnClick();
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        } else if (element->Tag == "h1") {
            ImGui::SetWindowFontScale(1.4f);
            ImGui::TextColored(ImVec4(
                element->Style.TextColor.r,
                element->Style.TextColor.g,
                element->Style.TextColor.b,
                element->Style.TextColor.a
            ), "%s", element->Text.c_str());
            ImGui::SetWindowFontScale(1.0f);
        } else if (element->Tag == "p" || element->Tag == "span") {
            ImGui::TextColored(ImVec4(
                element->Style.TextColor.r,
                element->Style.TextColor.g,
                element->Style.TextColor.b,
                element->Style.TextColor.a
            ), "%s", element->Text.c_str());
        } else {
            // Container <div>
            if (hasBg) {
                ImVec2 sz(element->Style.Width > 0 ? element->Style.Width : 0.0f,
                          element->Style.Height > 0 ? element->Style.Height : 0.0f);
                ImGui::BeginChild("DivBox", sz, true);
            }

            if (!element->Text.empty()) {
                ImGui::Text("%s", element->Text.c_str());
            }

            bool isRow = (element->Style.FlexDirection == UIFlexDirection::Row);
            for (size_t i = 0; i < element->Children.size(); ++i) {
                if (isRow && i > 0) ImGui::SameLine();
                RenderElement(element->Children[i]);
            }

            if (hasBg) {
                ImGui::EndChild();
            }
        }

        if (hasBg) {
            ImGui::PopStyleColor();
        }

        ImGui::PopID();
    }

} // namespace Engine
