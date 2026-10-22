/**
 * @file PhysicsComponent.hpp
 * @brief Dynamic physical properties component governing mass, forces, and velocities.
 */

#pragma once

#include "Component.hpp"
#include "core/Math.hpp"

namespace Engine {

    /**
     * @class PhysicsComponent
     * @brief Endows an actor with physical dynamic simulation properties.
     */
    class PhysicsComponent : public Component {
        GENERATED_BODY()
    public:
        PhysicsComponent();

        float Mass{1.0f};                         ///< Mass in kilograms (0 = static).
        bool bUseGravity{true};                   ///< Subject to downward gravity acceleration.
        bool bIsStatic{false};                    ///< Immovable static obstacle.
        float LinearDamping{0.01f};               ///< Drag coefficient slowing movement.
        float AngularDamping{0.05f};              ///< Rotational drag coefficient.

        glm::vec3 LinearVelocity{0.0f};           ///< Linear velocity (m/s).
        glm::vec3 AngularVelocity{0.0f};          ///< Angular velocity (deg/s).

        /**
         * @brief Applies an instantaneous impulse to change velocity.
         * @param impulse Direction and magnitude vector.
         */
        void AddImpulse(const glm::vec3& impulse) {
            if (!bIsStatic && Mass > 0.0f) {
                LinearVelocity += impulse / Mass;
            }
        }
    };

} // namespace Engine
