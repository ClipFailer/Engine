/**
 * @file OutlinerPanel.hpp
 * @brief Outliner / World hierarchy panel displaying scene actors with search and spawning.
 */

#pragma once

#include "scene/Scene.hpp"
#include <functional>

namespace Engine {

    /**
     * @class OutlinerPanel
     * @brief Outliner tree view of scene actors with creation, deletion, and selection.
     */
    class OutlinerPanel {
    public:
        OutlinerPanel() = default;

        /**
         * @brief Renders the outliner panel.
         * @param scene Active scene containing actors.
         */
        void Render(Scene& scene);

        [[nodiscard]] Actor* GetSelectedActor() const noexcept { return m_SelectedActor; }
        void SetSelectedActor(Actor* actor) noexcept { m_SelectedActor = actor; }

        std::function<void(Actor*)> OnActorDoubleClicked;

    private:
        Actor* m_SelectedActor{nullptr};
        char m_SearchFilter[128]{""};
    };

} // namespace Engine
