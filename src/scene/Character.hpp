/**
 * @file Character.hpp
 * @brief Base Character class extending Actor with built-in locomotion, jumping, and input controls.
 */

#pragma once

#include "Actor.hpp"
#include "MovementComponent.hpp"

namespace Engine {

    /**
     * @class Character
     * @brief Specialized Actor representing playable characters, NPCs, or pawns with movement capabilities.
     */
    class Character : public Actor {
        GENERATED_BODY()
    public:
        Character();
        explicit Character(std::string name);
        ~Character() override = default;

        void BeginPlay() override;
        void Tick(float deltaTime) override;

        /**
         * @brief Adds movement direction input scaled by world velocity.
         * @param direction World-space directional vector (e.g. forward, right).
         * @param scale Magnitude multiplier [-1.0, 1.0].
         */
        void AddMovementInput(const glm::vec3& direction, float scale = 1.0f);

        /**
         * @brief Initiates vertical jump impulse if character is grounded.
         */
        void Jump();

        float WalkSpeed{6.0f};          ///< Base movement velocity (units/sec).
        float JumpVelocity{7.5f};       ///< Instantaneous upward jump velocity.
        bool bIsGrounded{true};         ///< True if character is standing on a surface.
        float Health{100.0f};           ///< Current character hit points.
        float MaxHealth{100.0f};        ///< Maximum character health capacity.

    private:
        MovementComponent* m_MovementComponent{nullptr};
    };

} // namespace Engine
