/**
 * @file UIModule.cpp
 * @brief Implementation of UIModule.
 */

#include "UIModule.hpp"
#include "core/Log.hpp"

namespace Engine {

    UIModule::UIModule() = default;

    void UIModule::OnLoad() {
        LOG_INFO("UIModule loaded: HTML+CSS Game UI system online");

        // Load a default HUD layout sample
        std::string sampleHUD =
            "<div style=\"background-color: #1a1a24; padding: 12px; border-radius: 6px; width: 220px;\">"
            "  <h1 style=\"color: #4da6ff;\">Player HUD</h1>"
            "  <p style=\"color: #99e699;\">Health: 100%</p>"
            "  <p style=\"color: #ffcc66;\">Stamina: 85%</p>"
            "  <button id=\"btnAction\" style=\"background-color: #007acc; color: white; border-radius: 4px; height: 26px;\">Use Item</button>"
            "</div>";

        m_Document.LoadFromString(sampleHUD);

        if (auto btn = m_Document.GetRoot()->FindById("btnAction")) {
            btn->OnClick = []() {
                LOG_INFO("HTML UI Button clicked: Use Item!");
            };
        }
    }

    void UIModule::OnUpdate(float deltaTime) {
        (void)deltaTime;
    }

    void UIModule::OnUnload() {
        LOG_INFO("UIModule unloaded cleanly");
    }

} // namespace Engine
