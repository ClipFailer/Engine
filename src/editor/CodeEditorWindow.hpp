/**
 * @file CodeEditorWindow.hpp
 * @brief Integrated source code editor and file inspector for C++, headers, and scripts.
 */

#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace Engine {

    /**
     * @class CodeEditorWindow
     * @brief In-engine code editor enabling direct viewing, editing, and saving of .cpp/.hpp files,
     *        with direct one-click handoff to external editors like VS Code.
     */
    class CodeEditorWindow {
    public:
        CodeEditorWindow() = default;

        /**
         * @brief Opens a source or header file in the editor.
         * @param filePath Path to file on disk.
         */
        void OpenFile(const std::filesystem::path& filePath);

        /**
         * @brief Renders the code editor window.
         */
        void Render();

        [[nodiscard]] bool IsOpen() const noexcept { return m_bIsOpen; }
        void SetOpen(bool open) noexcept { m_bIsOpen = open; }

    private:
        bool m_bIsOpen{false};
        std::filesystem::path m_ActiveFilePath;
        std::string m_FileContent;
        std::vector<char> m_TextBuffer;
        bool m_bIsModified{false};
        std::string m_StatusMessage;
    };

} // namespace Engine
