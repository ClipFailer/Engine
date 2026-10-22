/**
 * @file DetailsPanel.hpp
 * @brief Details / Inspector panel displaying reflected properties and Unreal-style transform controls.
 */

#pragma once

#include "scene/Actor.hpp"
#include "reflection/Reflection.hpp"
#include "project/Project.hpp"

namespace Engine {

    /**
     * @class DetailsPanel
     * @brief Dynamic property inspector powered by runtime reflection and UE5 UI widgets.
     */
    class DetailsPanel {
    public:
        DetailsPanel() = default;

        /**
         * @brief Sets active project reference for generating component classes.
         * @param project Active project.
         */
        void SetProject(Project* project) noexcept { m_Project = project; }

        /**
         * @brief Renders the details inspector for the selected actor.
         * @param actor Currently selected actor (or nullptr).
         */
        void Render(Actor* actor);

    private:
        void DrawTransformControl(const std::string& label, glm::vec3& values, float resetValue = 0.0f);
        void DrawReflectedProperty(const PropertyDescriptor& prop, void* instance);

        Project* m_Project{nullptr};
        bool m_bShowCreateComponentModal{false};
        char m_NewComponentName[64]{"CustomGameplayComponent"};
    };

} // namespace Engine
