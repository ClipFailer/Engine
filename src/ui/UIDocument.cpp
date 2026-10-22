/**
 * @file UIDocument.cpp
 * @brief Implementation of HTML+CSS parsing and rendering.
 */

#include "UIDocument.hpp"
#include <imgui.h>
#include <regex>
#include <stack>
#include <algorithm>

namespace Engine {

    UIDocument::UIDocument() {
        m_Root = std::make_shared<UIElement>("root");
    }

    bool UIDocument::LoadFromString(const std::string& html, const std::string& css) {
        bool res = ParseHTML(html);
        if (!css.empty()) {
            ParseCSS(css);
        }
        return res;
    }

    bool UIDocument::ParseHTML(const std::string& html) {
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

        ApplyStyles(m_Root);
        return true;
    }

    bool UIDocument::ParseCSS(const std::string& css) {
        std::regex ruleRegex(R"(([a-zA-Z0-9_\-\.#]+)\s*\{([^}]*)\})");
        auto begin = std::sregex_iterator(css.begin(), css.end(), ruleRegex);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it) {
            std::string selector = (*it)[1].str();
            std::string decl = (*it)[2].str();

            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t\r\n"));
                s.erase(s.find_last_not_of(" \t\r\n") + 1);
            };
            trim(selector);
            trim(decl);

            m_CSSRules[selector] = decl;
        }

        ApplyStyles(m_Root);
        return true;
    }

    void UIDocument::ApplyStyles(const std::shared_ptr<UIElement>& element) {
        if (!element) return;

        // Apply matching tag selector
        if (auto it = m_CSSRules.find(element->Tag); it != m_CSSRules.end()) {
            element->Style.ParseInline(it->second);
        }

        // Apply class selector (.className)
        if (!element->ClassName.empty()) {
            if (auto it = m_CSSRules.find("." + element->ClassName); it != m_CSSRules.end()) {
                element->Style.ParseInline(it->second);
            }
        }

        // Apply ID selector (#id)
        if (!element->Id.empty()) {
            if (auto it = m_CSSRules.find("#" + element->Id); it != m_CSSRules.end()) {
                element->Style.ParseInline(it->second);
            }
        }

        for (const auto& child : element->Children) {
            ApplyStyles(child);
        }
    }

    void UIDocument::Render() {
        if (!m_Root) return;
        for (const auto& child : m_Root->Children) {
            RenderElement(child);
        }
    }

    void UIDocument::Render(ImDrawList* drawList, const ImVec2& screenPos, const ImVec2& size) {
        if (!m_Root || !drawList) return;
        ImVec2 curPos = screenPos;
        for (const auto& child : m_Root->Children) {
            RenderElementDrawList(child, drawList, curPos, size);
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

    void UIDocument::RenderElementDrawList(const std::shared_ptr<UIElement>& element, ImDrawList* drawList,
                                          ImVec2& curPos, const ImVec2& parentSize) {
        if (!element || !drawList) return;

        float w = element->Style.Width > 0.0f ? element->Style.Width : parentSize.x;
        float h = element->Style.Height > 0.0f ? element->Style.Height : 32.0f;

        ImVec2 boxMin = curPos;
        ImVec2 boxMax = ImVec2(boxMin.x + w, boxMin.y + h);

        // Background
        if (element->Style.BackgroundColor.a > 0.0f) {
            ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
                element->Style.BackgroundColor.r,
                element->Style.BackgroundColor.g,
                element->Style.BackgroundColor.b,
                element->Style.BackgroundColor.a
            ));
            drawList->AddRectFilled(boxMin, boxMax, bgCol, element->Style.BorderRadius);
        }

        // Border
        if (element->Style.BorderWidth > 0.0f && element->Style.BorderColor.a > 0.0f) {
            ImU32 borderCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
                element->Style.BorderColor.r,
                element->Style.BorderColor.g,
                element->Style.BorderColor.b,
                element->Style.BorderColor.a
            ));
            drawList->AddRect(boxMin, boxMax, borderCol, element->Style.BorderRadius, 0, element->Style.BorderWidth);
        }

        // Text
        if (!element->Text.empty()) {
            ImU32 textCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
                element->Style.TextColor.r,
                element->Style.TextColor.g,
                element->Style.TextColor.b,
                element->Style.TextColor.a
            ));
            ImVec2 textPos = ImVec2(boxMin.x + element->Style.PaddingLeft, boxMin.y + element->Style.PaddingTop);
            drawList->AddText(textPos, textCol, element->Text.c_str());
        }

        // Children
        bool isRow = (element->Style.FlexDirection == UIFlexDirection::Row);
        ImVec2 childPos = ImVec2(boxMin.x + element->Style.PaddingLeft,
                                 boxMin.y + (element->Text.empty() ? element->Style.PaddingTop : element->Style.PaddingTop + 20.0f));

        for (const auto& child : element->Children) {
            RenderElementDrawList(child, drawList, childPos, ImVec2(w - element->Style.PaddingLeft - element->Style.PaddingRight, h));
            if (isRow) {
                childPos.x += (child->Style.Width > 0 ? child->Style.Width : 60.0f) + 8.0f;
            } else {
                childPos.y += (child->Style.Height > 0 ? child->Style.Height : 32.0f) + 4.0f;
            }
        }

        if (!isRow) {
            curPos.y = std::max(curPos.y + h + element->Style.MarginBottom, childPos.y + element->Style.PaddingBottom);
        }
    }

} // namespace Engine
