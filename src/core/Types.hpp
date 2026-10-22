/**
 * @file Types.hpp
 * @brief Common types, aliases, and standard definitions for the engine.
 * @author Engine Developer
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <optional>
#include <span>
#include <string_view>

namespace Engine {

    using uint8  = std::uint8_t;
    using uint16 = std::uint16_t;
    using uint32 = std::uint32_t;
    using uint64 = std::uint64_t;

    using int8   = std::int8_t;
    using int16  = std::int16_t;
    using int32  = std::int32_t;
    using int64  = std::int64_t;

    /**
     * @brief Simulation states for the level and viewport.
     */
    enum class SimulationState : uint8 {
        Stopped = 0, ///< Editor mode, simulation is paused and physics are idle.
        Playing = 1, ///< Play in Editor (PIE), active simulation, actors tick and physics step.
        Paused  = 2  ///< Simulation temporarily paused, actors retain state.
    };

    /**
     * @brief Helper template for std::make_unique.
     */
    template<typename T, typename... Args>
    constexpr std::unique_ptr<T> CreateUnique(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    /**
     * @brief Helper template for std::make_shared.
     */
    template<typename T, typename... Args>
    constexpr std::shared_ptr<T> CreateShared(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace Engine
