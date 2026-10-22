/**
 * @file ContentBrowserPanel.hpp
 * @brief Unreal Engine 5 styled Content Browser panel with folder trees and asset actions.
 */

#pragma once

#include "project/Project.hpp"
#include <filesystem>

namespace Engine {

    class Scene;

    /**
     * @class ContentBrowserPanel
     * @brief File and asset browser supporting project exploration and C++ class creation.
     */
    class ContentBrowserPanel {
    public:
        explicit ContentBrowserPanel(Project& project);

        /**
         * @brief Renders the content browser panel.
         * @param scene Active scene pointer for loading scene assets.
         */
        void Render(Scene& scene);

    private:
        void DrawDirectoryTree(const std::filesystem::path& dir);
        void DrawBreadcrumbs();
        void DrawFileGrid(Scene& scene);

        Project& m_Project;
        std::filesystem::path m_RootPath;
        std::filesystem::path m_CurrentPath;

        char m_NewClassName[64]{"MyActor"};
        bool m_bShowCreateClassModal{false};
    };

} // namespace Engine
