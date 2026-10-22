/**
 * @file ProjectHubWindow.hpp
 * @brief Startup Welcome Launcher and Project Hub for browsing, opening, and creating projects.
 */

#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <functional>

namespace Engine {

    /**
     * @struct RecentProjectInfo
     * @brief Stored metadata for recently opened projects.
     */
    struct RecentProjectInfo {
        std::string Name;
        std::string Path;
        std::string LastOpened;
    };

    /**
     * @class ProjectHubWindow
     * @brief Welcome dialog shown at engine startup with recent projects and new project wizard.
     */
    class ProjectHubWindow {
    public:
        ProjectHubWindow();

        /**
         * @brief Renders the project hub launcher.
         */
        void Render();

        [[nodiscard]] bool IsOpen() const noexcept { return m_bIsOpen; }
        void Open() noexcept { m_bIsOpen = true; }
        void Close() noexcept { m_bIsOpen = false; }

        std::function<void(const std::filesystem::path&)> OnOpenProject;
        std::function<void(const std::filesystem::path&, const std::string&, bool)> OnCreateProject;

        void AddRecentProject(const std::string& name, const std::string& path);

    private:
        void LoadRecentProjects();
        void SaveRecentProjects();

        bool m_bIsOpen{true}; // Shown at engine startup!
        int m_SelectedTab{0}; // 0 = Recent, 1 = New Project

        std::vector<RecentProjectInfo> m_RecentProjects;

        char m_NewProjectName[64]{"MyNewGame"};
        char m_NewProjectPath[256]{"C:/Projects/Vulkan/MyNewGame"};
        bool m_bEmptyScene{true};
    };

} // namespace Engine
