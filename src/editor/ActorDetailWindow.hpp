/**
 * @file ActorDetailWindow.hpp
 * @brief Full-screen interactive 3D actor inspection window with mesh & collision bounds rendering.
 */

#pragma once

#include "scene/Actor.hpp"
#include <imgui.h>

namespace Engine {

    /**
     * @class ActorDetailWindow
     * @brief Dedicated full-screen inspector window opened via double-click on an Actor in Outliner or Viewport,
     *        featuring an interactive 3D scene preview showing the object, mesh, and real-time collision wireframe.
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
        void Close() noexcept { m_bIsOpen = false; }
        [[nodiscard]] Actor* GetTargetActor() const noexcept { return m_TargetActor; }

    private:
        void Draw3DPreview(ImVec2 canvasSize);
        void DrawInspectorPanel();

        Actor* m_TargetActor{nullptr};
        bool m_bIsOpen{false};

        // Orbit camera state for 3D inspection
        float m_CamYaw{35.0f};
        float m_CamPitch{20.0f};
        float m_CamDistance{3.5f};
    };

} // namespace Engine
