/**
 * @file UIDocument.hpp
 * @brief HTML & CSS document container with parsing and interactive rendering.
 */

#pragma once

#include "UIElement.hpp"
#include <string>
#include <memory>

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
         * @return True if parsed.
         */
        bool LoadFromString(const std::string& html);

        /**
         * @brief Renders the UI document into the active ImGui window/viewport.
         */
        void Render();

        [[nodiscard]] std::shared_ptr<UIElement> GetRoot() const noexcept { return m_Root; }

    private:
        void RenderElement(const std::shared_ptr<UIElement>& element);

        std::shared_ptr<UIElement> m_Root;
    };

} // namespace Engine
