/**
 * @file MovementComponent.hpp
 * @brief Kinematic movement component handling character and pawn locomotion.
 */

#pragma once

#include "Component.hpp"
#include "core/Math.hpp"

namespace Engine {

    /**
     * @class MovementComponent
     * @brief Translates and accelerates an actor through directional input vectors.
     */
    class MovementComponent : public Component {
        GENERATED_BODY()
    public:
        MovementComponent();

        void Tick(float deltaTime) override;

        /**
         * @brief Adds a directional input intention vector.
         * @param direction Normalized movement direction.
         * @param scale Magnitude multiplier.
         */
        void AddInputVector(const glm::vec3& direction, float scale = 1.0f);

        float MaxSpeed{6.0f};                      ///< Maximum movement speed (m/s).
        float Acceleration{25.0f};                 ///< Speed ramp-up rate.
        float Deceleration{30.0f};                 ///< Braking deceleration rate.

        glm::vec3 Velocity{0.0f};                  ///< Current translation velocity.

    private:
        glm::vec3 m_PendingInput{0.0f};
    };

} // namespace Engine
