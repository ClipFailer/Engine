/**
 * @file IModule.hpp
 * @brief Base interface for engine subsystem modules.
 */

#pragma once

#include "core/Types.hpp"
#include <string_view>

namespace Engine {

    /**
     * @class IModule
     * @brief Abstract base class for extensible engine plugins and modules.
     */
    class IModule {
    public:
        virtual ~IModule() = default;

        /**
         * @brief Called when the module is loaded and registered.
         */
        virtual void OnLoad() = 0;

        /**
         * @brief Called every frame to update module logic.
         * @param deltaTime Seconds elapsed since last frame.
         */
        virtual void OnUpdate(float deltaTime) { (void)deltaTime; }

        /**
         * @brief Called when the module is shutting down.
         */
        virtual void OnUnload() = 0;

        /**
         * @brief Returns human-readable unique module name.
         * @return Name string view.
         */
        [[nodiscard]] virtual std::string_view GetName() const = 0;

        [[nodiscard]] virtual bool IsEnabled() const noexcept { return m_bEnabled; }
        virtual void SetEnabled(bool enabled) noexcept { m_bEnabled = enabled; }

    protected:
        bool m_bEnabled{true};
    };

} // namespace Engine
