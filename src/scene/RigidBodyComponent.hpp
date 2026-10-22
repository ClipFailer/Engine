/**
 * @file RigidBodyComponent.hpp
 * @brief Dynamic rigid body physics component enabling forces, gravity, and collisions.
 */

#pragma once

#include "Component.hpp"
#include "core/Math.hpp"

namespace Engine {

    enum class ColliderType : uint8 {
        Box = 0,
        Sphere = 1
    };

    /**
     * @class RigidBodyComponent
     * @brief Physics body subject to forces, gravity, and restitution collisions.
     */
    class RigidBodyComponent : public Component {
        GENERATED_BODY()
    public:
        RigidBodyComponent();

        float Mass{1.0f};                         ///< Mass in kilograms (0 = infinite / static).
        bool bUseGravity{true};                   ///< Apply scene gravity acceleration.
        bool bIsStatic{false};                    ///< Static obstacle unaffected by forces.
        float Restitution{0.6f};                  ///< Bounciness coefficient [0..1].
        float Friction{0.2f};                     ///< Surface friction coefficient.
        ColliderType Shape{ColliderType::Box};    ///< Collision boundary shape.
        glm::vec3 LinearVelocity{0.0f};           ///< Current 3D linear velocity (m/s).
        glm::vec3 AngularVelocity{0.0f};          ///< Current angular velocity (deg/s).
        glm::vec3 ColliderDimensions{1.0f};       ///< Half-extents for Box or Radius for Sphere.

        /**
         * @brief Applies a directional impulse force to the body.
         * @param impulse Impulse vector.
         */
        void AddImpulse(const glm::vec3& impulse) {
            if (!bIsStatic && Mass > 0.0f) {
                LinearVelocity += impulse / Mass;
            }
        }
    };

} // namespace Engine
