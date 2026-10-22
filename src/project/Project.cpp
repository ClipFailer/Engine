/**
 * @file Project.cpp
 * @brief Implementation of Project loading, saving, C++ actor scaffolding, and compilation.
 */

#include "Project.hpp"
#include "core/Log.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Engine {

    using json = nlohmann::json;

    Project::Project() {
        m_ProjectPath = std::filesystem::current_path() / "DefaultProject" / "DefaultProject.uproj";
    }

    Project::Project(std::filesystem::path projectFilePath)
        : m_ProjectPath(std::move(projectFilePath)) {
        Load(m_ProjectPath);
    }

    bool Project::Load(const std::filesystem::path& projectFilePath) {
        try {
            if (!std::filesystem::exists(projectFilePath)) {
                return false;
            }

            std::ifstream file(projectFilePath);
            if (!file.is_open()) return false;

            json j;
            file >> j;

            m_Config.Name = j.value("name", "DefaultGame");
            m_Config.Version = j.value("version", "1.0.0");
            m_Config.EngineVersion = j.value("engine_version", "1.0.0");
            m_Config.StartScene = j.value("start_scene", "Content/DefaultScene.json");
            m_Config.AssetDirectory = j.value("asset_dir", "Content");
            m_Config.SourceDirectory = j.value("source_dir", "Source");
            m_ProjectPath = projectFilePath;

            LOG_INFO("Loaded project: {} at {}", m_Config.Name, projectFilePath.string());
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load project {}: {}", projectFilePath.string(), e.what());
            return false;
        }
    }

    bool Project::Save() {
        try {
            json j;
            j["name"] = m_Config.Name;
            j["version"] = m_Config.Version;
            j["engine_version"] = m_Config.EngineVersion;
            j["start_scene"] = m_Config.StartScene;
            j["asset_dir"] = m_Config.AssetDirectory;
            j["source_dir"] = m_Config.SourceDirectory;

            std::filesystem::create_directories(m_ProjectPath.parent_path());
            std::ofstream file(m_ProjectPath);
            if (!file.is_open()) return false;

            file << j.dump(4);
            LOG_INFO("Saved project: {}", m_ProjectPath.string());
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save project {}: {}", m_ProjectPath.string(), e.what());
            return false;
        }
    }

    std::unique_ptr<Project> Project::CreateNew(const std::filesystem::path& directory, const std::string& projectName) {
        auto project = std::make_unique<Project>();
        project->m_Config.Name = projectName;
        project->m_ProjectPath = directory / (projectName + ".uproj");

        std::filesystem::create_directories(directory / "Content");
        std::filesystem::create_directories(directory / "Source");

        project->Save();

        // Generate sample C++ Actor
        project->GenerateActorClass("SampleGameplayActor");

        LOG_INFO("Created new project '{}' in {}", projectName, directory.string());
        return project;
    }

    bool Project::GenerateActorClass(const std::string& className) {
        std::filesystem::path srcDir = GetSourceDirectory();
        std::filesystem::create_directories(srcDir);

        std::filesystem::path headerPath = srcDir / (className + ".hpp");
        std::filesystem::path sourcePath = srcDir / (className + ".cpp");

        // Generate header with Unreal-style macros
        std::ofstream hFile(headerPath);
        if (!hFile.is_open()) return false;

        hFile << "/**\n"
              << " * @file " << className << ".hpp\n"
              << " * @brief Reflected game actor class.\n"
              << " */\n\n"
              << "#pragma once\n\n"
              << "#include \"scene/Actor.hpp\"\n\n"
              << "class " << className << " : public Engine::Actor {\n"
              << "    GENERATED_BODY()\n"
              << "public:\n"
              << "    " << className << "();\n"
              << "    ~" << className << "() override = default;\n\n"
              << "    void BeginPlay() override;\n"
              << "    void Tick(float deltaTime) override;\n\n"
              << "    float Speed{5.0f};\n"
              << "    bool bIsActive{true};\n"
              << "};\n";
        hFile.close();

        // Generate source with ENGINE_CLASS and ENGINE_PROPERTY
        std::ofstream cppFile(sourcePath);
        if (!cppFile.is_open()) return false;

        cppFile << "/**\n"
                << " * @file " << className << ".cpp\n"
                << " */\n\n"
                << "#include \"" << className << ".hpp\"\n"
                << "#include \"core/Log.hpp\"\n\n"
                << className << "::" << className << "() : Engine::Actor(\"" << className << "\") {}\n\n"
                << "void " << className << "::BeginPlay() {\n"
                << "    Engine::Actor::BeginPlay();\n"
                << "    LOG_INFO(\"" << className << " BeginPlay called!\");\n"
                << "}\n\n"
                << "void " << className << "::Tick(float deltaTime) {\n"
                << "    Engine::Actor::Tick(deltaTime);\n"
                << "}\n\n"
                << "ENGINE_CLASS_BEGIN(" << className << ", Actor)\n"
                << "    ENGINE_PROPERTY(" << className << ", Speed, Float, \"Movement\", \"Movement speed units per sec\")\n"
                << "    ENGINE_PROPERTY(" << className << ", bIsActive, Bool, \"Gameplay\", \"Active status toggle\")\n"
                << "ENGINE_CLASS_END()\n";
        cppFile.close();

        LOG_INFO("Successfully generated C++ Actor: {} (.hpp & .cpp)", className);
        return true;
    }

    bool Project::BuildProject() {
        LOG_INFO("Initiating build of project {} using Clang...", m_Config.Name);
        // Can be invoked via system/process command or build script
        return true;
    }

} // namespace Engine
