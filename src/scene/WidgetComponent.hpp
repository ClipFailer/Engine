/**
 * @file WidgetComponent.hpp
 * @brief UI Widget Component enabling 3D world-space floating UI or 2D player screen HUD via HTML+CSS.
 */

#pragma once

#include "Component.hpp"
#include "ui/UIDocument.hpp"
#include <imgui.h>
#include <string>

namespace Engine {

    class Camera;

    /**
     * @class WidgetComponent
     * @brief Component that attaches an interactive HTML+CSS UI widget to an Actor or displays as screen HUD.
     */
    class WidgetComponent : public Component {
        GENERATED_BODY()
    public:
        WidgetComponent();
        explicit WidgetComponent(std::string htmlMarkup, std::string cssStyles = "");
        ~WidgetComponent() override = default;

        /**
         * @brief Loads HTML & CSS from disk.
         * @param filePath Path to .ui or .html asset.
         */
        bool LoadFromFile(const std::string& filePath);

        /**
         * @brief Renders the widget using the active camera and ImGui DrawList.
         * @param actorPos World position of the owning actor.
         * @param camera Active scene camera.
         * @param drawList ImGui DrawList for rendering.
         * @param viewportPos Screen position of the viewport.
         * @param viewportSize Dimensions of the viewport.
         */
        void Render(const glm::vec3& actorPos, const Camera& camera, ImDrawList* drawList,
                    ImVec2 viewportPos, ImVec2 viewportSize);

        std::string HTMLContent{"<div class='hud-box'><h1>Player Name</h1><p>Health: 100/100</p></div>"};
        std::string CSSContent{".hud-box { background: #1a1e26; color: #40c057; padding: 8px; border-radius: 4px; }"};

        bool bIsWorldSpace{true};        ///< If true, renders floating in 3D above actor; if false, screen HUD.
        glm::vec2 DrawSize{200.0f, 60.0f}; ///< Canvas size for UI rendering.
        glm::vec3 Offset{0.0f, 1.5f, 0.0f};///< World position offset above actor.

    private:
        UIDocument m_Document;
        bool m_bIsDirty{true};
    };

} // namespace Engine
