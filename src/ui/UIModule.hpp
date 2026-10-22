/**
 * @file UIModule.hpp
 * @brief HTML+CSS Game UI module implementation.
 */

#pragma once

#include "module/IModule.hpp"
#include "UIDocument.hpp"

namespace Engine {

    /**
     * @class UIModule
     * @brief Engine module providing HTML+CSS UI runtime for game viewports and HUD overlays.
     */
    class UIModule : public IModule {
    public:
        UIModule();
        ~UIModule() override = default;

        void OnLoad() override;
        void OnUpdate(float deltaTime) override;
        void OnUnload() override;

        [[nodiscard]] std::string_view GetName() const override { return "UIModule"; }

        /**
         * @brief Returns active game HUD document.
         * @return Reference to UIDocument.
         */
        [[nodiscard]] UIDocument& GetDocument() noexcept { return m_Document; }

    private:
        UIDocument m_Document;
    };

} // namespace Engine
