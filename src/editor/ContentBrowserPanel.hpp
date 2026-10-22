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
        std::function<void(const std::filesystem::path&)> OnOpenScene;
        std::function<void(const std::string&)> OnSpawnActorFromClass;
        std::function<void(const std::string&)> OnActorAssetDoubleClicked;
        std::function<void(const std::filesystem::path&)> OnUIDoubleClicked;
        std::function<void(const std::string&)> OnAttachComponentToSelected;

    private:
        void DrawDirectoryTree(const std::filesystem::path& dir);
        void DrawBreadcrumbs();
        void DrawFileGrid(Scene& scene);

        Project* m_Project{nullptr};
        std::filesystem::path m_RootPath;
        std::filesystem::path m_CurrentPath;

        // Create Class Wizard with Parent Class Search
        bool m_bShowCreateClassWizard{false};
        char m_WizardClassName[64]{"MyNewEntity"};
        char m_WizardParentClass[64]{"Actor"};
        char m_ParentSearchFilter[64]{""};

        // New Folder Modal
        bool m_bShowNewFolderModal{false};
        char m_NewFolderName[64]{"NewFolder"};

        char m_RenameBuffer[128]{""};
        std::filesystem::path m_ItemToRename;
        bool m_bShowRenameModal{false};
    };

} // namespace Engine
