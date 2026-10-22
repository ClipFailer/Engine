/**
 * @file ActorDetailWindow.hpp
 * @brief Modal / floating inspector window for deep Actor configuration, meshes, tags, and collision fitting.
 */

#pragma once

#include "scene/Actor.hpp"

namespace Engine {

    /**
     * @class ActorDetailWindow
     * @brief Dedicated inspector window opened via double-click on an Actor in Outliner or Viewport.
     */
    class ActorDetailWindow {
    public:
        ActorDetailWindow() = default;

        /**
         * @brief Opens the window targeting a specific Actor.
         * @param actor Actor to inspect and edit.
         */
        void Open(Actor* actor) {
            m_TargetActor = actor;
            m_bIsOpen = (actor != nullptr);
        }

        /**
         * @brief Renders the Actor detail window.
         */
        void Render();

        [[nodiscard]] bool IsOpen() const noexcept { return m_bIsOpen; }
        [[nodiscard]] Actor* GetTargetActor() const noexcept { return m_TargetActor; }

    private:
        Actor* m_TargetActor{nullptr};
        bool m_bIsOpen{false};
    };

} // namespace Engine
