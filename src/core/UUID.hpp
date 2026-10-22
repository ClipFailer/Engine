/**
 * @file UUID.hpp
 * @brief 64-bit unique identifier generation and hashing.
 */

#pragma once

#include "Types.hpp"
#include <random>

namespace Engine {

    /**
     * @class UUID
     * @brief Represents a globally unique 64-bit identifier.
     */
    class UUID {
    public:
        /**
         * @brief Generates a random 64-bit UUID.
         */
        UUID() {
            static std::random_device rd;
            static std::mt19937_64 engine(rd());
            static std::uniform_int_distribution<uint64> distribution;
            m_Value = distribution(engine);
        }

        /**
         * @brief Constructs a UUID from an explicit integer value.
         * @param value Specific 64-bit identifier.
         */
        constexpr UUID(uint64 value) : m_Value(value) {}

        /**
         * @brief Copy constructor.
         */
        constexpr UUID(const UUID&) = default;

        /**
         * @brief Copy assignment operator.
         */
        constexpr UUID& operator=(const UUID&) = default;

        /**
         * @brief Implicit conversion to uint64.
         */
        constexpr operator uint64() const { return m_Value; }

        /**
         * @brief Comparison operator for equality.
         */
        constexpr bool operator==(const UUID& other) const { return m_Value == other.m_Value; }

        /**
         * @brief Comparison operator for inequality.
         */
        constexpr bool operator!=(const UUID& other) const { return m_Value != other.m_Value; }

        /**
         * @brief Returns the raw 64-bit value.
         * @return Raw uint64 identifier.
         */
        [[nodiscard]] constexpr uint64 Get() const noexcept { return m_Value; }

    private:
        uint64 m_Value{0};
    };

} // namespace Engine

namespace std {
    template<>
    struct hash<Engine::UUID> {
        std::size_t operator()(const Engine::UUID& uuid) const noexcept {
            return std::hash<Engine::uint64>()(static_cast<Engine::uint64>(uuid));
        }
    };
}
