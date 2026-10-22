/**
 * @file Project.hpp
 * @brief Project definition, configuration, C++ template code generator, and build pipeline.
 */

#pragma once

#include "core/Types.hpp"
#include <string>
#include <filesystem>

namespace Engine {

    /**
     * @struct ProjectConfig
     * @brief Configuration manifest saved in .uproj file.
     */
    struct ProjectConfig {
        std::string Name{"DefaultGame"};
        std::string Version{"1.0.0"};
        std::string EngineVersion{"1.0.0"};
        std::string StartScene{"Content/DefaultScene.json"};
        std::string AssetDirectory{"Content"};
        std::string SourceDirectory{"Source"};
    };

    /**
     * @class Project
     * @brief Manages active game project workspace, files, and C++ gameplay actor creation.
     */
    class Project {
    public:
        Project();
        explicit Project(std::filesystem::path projectFilePath);

        /**
         * @brief Loads project from .uproj file.
         * @param projectFilePath Absolute or relative path to .uproj.
         * @return True if loaded.
         */
        bool Load(const std::filesystem::path& projectFilePath);

        /**
         * @brief Saves project manifest to .uproj file.
         * @return True if saved.
         */
        bool Save();

        /**
         * @brief Creates a new engine project at the specified directory.
         * @param directory Target folder.
         * @param projectName Name of new project.
         * @return Pointer to created Project.
         */
        static std::unique_ptr<Project> CreateNew(const std::filesystem::path& directory, const std::string& projectName);

        /**
         * @brief Generates boilerplate C++ header and source files for a new reflected Actor class.
         * @param className Name of C++ class (e.g., "PlayerCharacter").
         * @return True if generated.
         */
        bool GenerateActorClass(const std::string& className);

        /**
         * @brief Compiles C++ game module using CMake and Clang.
         * @return True if build succeeded.
         */
        bool BuildProject();

        [[nodiscard]] const ProjectConfig& GetConfig() const noexcept { return m_Config; }
        [[nodiscard]] const std::filesystem::path& GetProjectPath() const noexcept { return m_ProjectPath; }
        [[nodiscard]] std::filesystem::path GetContentDirectory() const { return m_ProjectPath.parent_path() / m_Config.AssetDirectory; }
        [[nodiscard]] std::filesystem::path GetSourceDirectory() const { return m_ProjectPath.parent_path() / m_Config.SourceDirectory; }

    private:
        ProjectConfig m_Config;
        std::filesystem::path m_ProjectPath;
    };

} // namespace Engine
