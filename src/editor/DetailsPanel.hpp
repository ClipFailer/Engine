/**
 * @file DetailsPanel.hpp
 * @brief Details / Inspector panel displaying reflected properties and Unreal-style transform controls.
 */

#pragma once

#include "scene/Actor.hpp"
#include "reflection/Reflection.hpp"

namespace Engine {

    /**
     * @class DetailsPanel
     * @brief Dynamic property inspector powered by runtime reflection and UE5 UI widgets.
     */
    class DetailsPanel {
    public:
        DetailsPanel() = default;

        /**
         * @brief Renders the details inspector for the selected actor.
         * @param actor Currently selected actor (or nullptr).
         */
        void Render(Actor* actor);

    private:
        void DrawTransformControl(const std::string& label, glm::vec3& values, float resetValue = 0.0f);
        void DrawReflectedProperty(const PropertyDescriptor& prop, void* instance);
    };

} // namespace Engine
