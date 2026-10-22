/**
 * @file WidgetComponent.cpp
 * @brief Implementation of WidgetComponent HTML+CSS world & screen rendering.
 */

#include "WidgetComponent.hpp"
#include "Camera.hpp"
#include <fstream>
#include <sstream>

namespace Engine {

    WidgetComponent::WidgetComponent() : Component("WidgetComponent") {
        m_Document.ParseHTML(HTMLContent);
        m_Document.ParseCSS(CSSContent);
    }

    WidgetComponent::WidgetComponent(std::string htmlMarkup, std::string cssStyles)
        : Component("WidgetComponent"), HTMLContent(std::move(htmlMarkup)), CSSContent(std::move(cssStyles)) {
        m_Document.ParseHTML(HTMLContent);
        m_Document.ParseCSS(CSSContent);
    }

    bool WidgetComponent::LoadFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        std::stringstream buffer;
        buffer << file.rdbuf();
        HTMLContent = buffer.str();
        m_Document.ParseHTML(HTMLContent);
        m_Document.ParseCSS(CSSContent);
        m_bIsDirty = false;
        return true;
    }

    void WidgetComponent::Render(const glm::vec3& actorPos, const Camera& camera, ImDrawList* drawList,
                                 ImVec2 viewportPos, ImVec2 viewportSize) {
        if (!drawList || viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) return;

        if (m_bIsDirty) {
            m_Document.ParseHTML(HTMLContent);
            m_Document.ParseCSS(CSSContent);
            m_bIsDirty = false;
        }

        ImVec2 screenPos;

        if (bIsWorldSpace) {
            glm::vec3 worldTarget = actorPos + Offset;
            // Standard perspective projection without clip flip
            float aspect = viewportSize.x / viewportSize.y;
            glm::mat4 proj = glm::perspective(glm::radians(camera.GetFOV()), aspect, camera.GetNearClip(), camera.GetFarClip());
            glm::mat4 vp = proj * camera.GetViewMatrix();

            glm::vec4 clip = vp * glm::vec4(worldTarget, 1.0f);
            if (clip.w <= 0.05f) return; // Behind camera

            glm::vec3 ndc = glm::vec3(clip) / clip.w;
            if (ndc.z < -1.0f || ndc.z > 1.0f) return;

            screenPos.x = viewportPos.x + (ndc.x * 0.5f + 0.5f) * viewportSize.x - (DrawSize.x * 0.5f);
            screenPos.y = viewportPos.y + (0.5f - ndc.y * 0.5f) * viewportSize.y - (DrawSize.y * 0.5f);
        } else {
            // Screen HUD: anchored to top-left with offset
            screenPos.x = viewportPos.x + 20.0f;
            screenPos.y = viewportPos.y + 20.0f;
        }

        m_Document.Render(drawList, screenPos, ImVec2(DrawSize.x, DrawSize.y));
    }

    ENGINE_CLASS_BEGIN(WidgetComponent, Component)
        ENGINE_PROPERTY(WidgetComponent, HTMLContent, String, "UI", "HTML structure markup")
        ENGINE_PROPERTY(WidgetComponent, CSSContent, String, "UI", "CSS style rules")
        ENGINE_PROPERTY(WidgetComponent, bIsWorldSpace, Bool, "UI", "Renders floating in 3D world vs screen HUD")
    ENGINE_CLASS_END()

} // namespace Engine
