/**
 * @file UIDocument.hpp
 * @brief HTML & CSS document container with parsing and interactive rendering.
 */

#pragma once

#include "UIElement.hpp"
#include <string>
#include <memory>
#include <unordered_map>

struct ImDrawList;
struct ImVec2;

namespace Engine {

    /**
     * @class UIDocument
     * @brief Parses and hosts HTML+CSS documents for in-game HUDs and simulation UI.
     */
    class UIDocument {
    public:
        UIDocument();
        ~UIDocument() = default;

        /**
         * @brief Parses an HTML+CSS markup string into a DOM tree.
         * @param html Markup string with tags and inline styles.
         * @param css Optional CSS style rules.
         * @return True if parsed.
         */
        bool LoadFromString(const std::string& html, const std::string& css = "");

        /**
         * @brief Parses HTML markup.
         */
        bool ParseHTML(const std::string& html);

        /**
         * @brief Parses CSS style rules and applies them to existing DOM elements.
         */
        bool ParseCSS(const std::string& css);

        /**
         * @brief Renders the UI document into the active ImGui window.
         */
        void Render();

        /**
         * @brief Renders the UI document into a specific draw list at the given screen coordinates.
         * @param drawList ImDrawList pointer (e.g. from ImGui::GetWindowDrawList()).
         * @param screenPos Top-left screen position.
         * @param size Available width and height.
         */
        void Render(ImDrawList* drawList, const ImVec2& screenPos, const ImVec2& size);

        [[nodiscard]] std::shared_ptr<UIElement> GetRoot() const noexcept { return m_Root; }

    private:
        void ApplyStyles(const std::shared_ptr<UIElement>& element);
        void RenderElement(const std::shared_ptr<UIElement>& element);
        void RenderElementDrawList(const std::shared_ptr<UIElement>& element, ImDrawList* drawList,
                                   ImVec2& curPos, const ImVec2& parentSize);

        std::shared_ptr<UIElement> m_Root;
        std::unordered_map<std::string, std::string> m_CSSRules;
    };

} // namespace Engine
