/**
 * @file MovementComponent.cpp
 * @brief Implementation and reflection metadata for MovementComponent.
 */

#include "MovementComponent.hpp"
#include "Actor.hpp"

namespace Engine {

    MovementComponent::MovementComponent()
        : Component("MovementComponent") {}

    void MovementComponent::AddInputVector(const glm::vec3& direction, float scale) {
        m_PendingInput += direction * scale;
    }

    void MovementComponent::Tick(float deltaTime) {
        if (!m_Owner) return;

        glm::vec3 desiredVelocity = glm::vec3(0.0f);
        if (glm::length(m_PendingInput) > 0.0001f) {
            desiredVelocity = glm::normalize(m_PendingInput) * MaxSpeed;
        }

        // Smooth acceleration / deceleration
        float rate = (glm::length(desiredVelocity) > 0.0f) ? Acceleration : Deceleration;
        Velocity = glm::mix(Velocity, desiredVelocity, glm::clamp(rate * deltaTime, 0.0f, 1.0f));

        // Apply movement to owner actor transform
        m_Owner->Transform.Position += Velocity * deltaTime;

        // Reset frame input
        m_PendingInput = glm::vec3(0.0f);
    }

    ENGINE_CLASS_BEGIN(MovementComponent, Component)
        ENGINE_PROPERTY(MovementComponent, MaxSpeed, Float, "Movement", "Maximum movement velocity")
        ENGINE_PROPERTY(MovementComponent, Acceleration, Float, "Movement", "Acceleration rate")
        ENGINE_PROPERTY(MovementComponent, Deceleration, Float, "Movement", "Braking deceleration rate")
        ENGINE_PROPERTY(MovementComponent, Velocity, Vec3, "Movement", "Current velocity vector")
    ENGINE_CLASS_END()

} // namespace Engine
