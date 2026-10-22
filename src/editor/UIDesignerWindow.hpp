/**
 * @file UIDesignerWindow.hpp
 * @brief Interactive in-engine UI Designer and live preview tool for HTML+CSS widgets and HUD.
 */

#pragma once

#include "ui/UIDocument.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace Engine {

    class Actor;

    /**
     * @class UIDesignerWindow
     * @brief Dedicated UI authoring window supporting live HTML+CSS editing and instant visual preview.
     */
    class UIDesignerWindow {
    public:
        UIDesignerWindow();

        /**
         * @brief Opens the designer with a specific widget file.
         * @param filePath Path to .ui or .html asset.
         */
        void OpenFile(const std::filesystem::path& filePath);

        /**
         * @brief Renders the UI designer window.
         * @param selectedActor Active selected actor to allow attaching the widget.
         */
        void Render(Actor* selectedActor = nullptr);

        [[nodiscard]] bool IsOpen() const noexcept { return m_bIsOpen; }
        void SetOpen(bool open) noexcept { m_bIsOpen = open; }

    private:
        bool m_bIsOpen{false};
        std::filesystem::path m_ActiveFilePath;

        std::string m_HTMLSource;
        std::string m_CSSSource;

        std::vector<char> m_HTMLBuffer;
        std::vector<char> m_CSSBuffer;

        UIDocument m_Document;
        bool m_bNeedsRebuild{true};
    };

} // namespace Engine
