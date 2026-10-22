/**
 * @file ContentBrowserPanel.hpp
 * @brief Unreal Engine 5 styled Content Browser panel with folder trees and asset actions.
 */

#pragma once

#include "project/Project.hpp"
#include <filesystem>
#include <functional>

namespace Engine {

    class Scene;

    /**
     * @class ContentBrowserPanel
     * @brief File and asset browser supporting project exploration, file management, and C++ class creation.
     */
    class ContentBrowserPanel {
    public:
        explicit ContentBrowserPanel(Project& project);

        /**
         * @brief Updates active project reference and resets root paths.
         * @param project New active project.
         */
        void SetProject(Project& project);

        /**
         * @brief Renders the content browser panel.
         * @param scene Active scene pointer for loading scene assets and spawning actors.
         */
        void Render(Scene& scene);

        std::function<void(const std::filesystem::path&)> OnFileDoubleClicked;
        std::function<void(const std::string&)> OnSpawnActorFromClass;

    private:
        void DrawDirectoryTree(const std::filesystem::path& dir);
        void DrawBreadcrumbs();
        void DrawFileGrid(Scene& scene);

        Project* m_Project{nullptr};
        std::filesystem::path m_RootPath;
        std::filesystem::path m_CurrentPath;

        char m_NewClassName[64]{"MyActor"};
        bool m_bShowCreateClassModal{false};

        char m_NewComponentName[64]{"MyCustomComponent"};
        bool m_bShowCreateComponentModal{false};

        char m_RenameBuffer[128]{""};
        std::filesystem::path m_ItemToRename;
        bool m_bShowRenameModal{false};
    };

} // namespace Engine
