/**
 * @file SettingsDialogs.hpp
 * @brief Modal windows for Project Settings and Engine Configuration.
 */

#pragma once

#include "project/Project.hpp"
#include "physics/PhysicsWorld.hpp"

namespace Engine {

    /**
     * @class SettingsDialogs
     * @brief Manages modals for configuring engine parameters and project metadata.
     */
    class SettingsDialogs {
    public:
        SettingsDialogs() = default;

        void OpenProjectSettings() { m_bShowProjectSettings = true; }
        void OpenEngineSettings() { m_bShowEngineSettings = true; }

        /**
         * @brief Renders dialogs if opened.
         * @param project Active project.
         * @param physicsWorld Active physics world.
         */
        void Render(Project& project, PhysicsWorld& physicsWorld);

    private:
        bool m_bShowProjectSettings{false};
        bool m_bShowEngineSettings{false};

        float m_MouseSensitivity{0.15f};
        float m_CameraBaseSpeed{5.0f};
        bool m_bVSync{true};
    };

} // namespace Engine
