/**
 * @file ActorDetailWindow.hpp
 * @brief Full-screen interactive 3D actor inspection window with mesh & collision bounds rendering.
 */

#pragma once

#include "scene/Actor.hpp"
#include <imgui.h>

namespace Engine {

    class Scene;

    /**
     * @class ActorDetailWindow
     * @brief Dedicated full-screen inspector window opened via double-click on an Actor in Outliner or Viewport,
     *        featuring an interactive 3D scene preview showing the object, mesh, and real-time collision wireframe.
     */
    class ActorDetailWindow {
    public:
        ActorDetailWindow() = default;

        /**
         * @brief Opens the window targeting a specific Actor and optional asset path.
         * @param actor Actor to inspect and edit.
         * @param assetPath Explicit file path to .actor preset/blueprint.
         */
        void Open(Actor* actor, const std::string& assetPath = "");

        /**
         * @brief Renders the Actor detail window.
         * @param scene Active scene to validate actor existence.
         */
        void Render(const Scene& scene);

        [[nodiscard]] bool IsOpen() const noexcept { return m_bIsOpen; }
        void Close() noexcept;
        [[nodiscard]] Actor* GetTargetActor() const noexcept { return m_TargetActor; }

        /**
         * @brief Persists current actor properties and components to its associated .actor file.
         * @return True if saved successfully.
         */
        bool SaveCurrentAsset();

        void SetProjectContentPath(const std::filesystem::path& path) noexcept {
            m_ProjectContentPath = path;
        }

    private:
        void Draw3DPreview(ImVec2 canvasSize);
        void DrawInspectorPanel();

        Actor* m_TargetActor{nullptr};
        std::string m_AssociatedAssetPath{};
        std::filesystem::path m_ProjectContentPath{};
        bool m_bIsOpen{false};
        bool m_bJustOpened{false};
        bool m_bDirty{false};
        float m_SaveNotificationTimer{0.0f};

        // Orbit camera state for 3D inspection
        float m_CamYaw{35.0f};
        float m_CamPitch{20.0f};
        float m_CamDistance{3.5f};
    };

} // namespace Engine
