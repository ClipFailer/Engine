/**
 * @file Character.cpp
 * @brief Implementation of Character gameplay mechanics and reflection registration.
 */

#include "Character.hpp"
#include "PhysicsComponent.hpp"
#include "CollisionComponent.hpp"
#include "StaticMeshComponent.hpp"
#include "core/Log.hpp"

namespace Engine {

    Character::Character() : Character("Character") {}

    Character::Character(std::string name) : Actor(std::move(name), true) {
        m_MovementComponent = AddComponent<MovementComponent>();
        m_MovementComponent->MaxSpeed = WalkSpeed;

        // Default Character visual representation and collision
        if (auto mesh = GetComponent<StaticMeshComponent>()) {
            mesh->MeshType = PrimitiveType::Cube; // Can represent humanoid box/capsule
        }
        if (auto coll = GetComponent<CollisionComponent>()) {
            coll->Shape = CollisionShape::Box;
            coll->BoxHalfExtents = glm::vec3(0.4f, 0.9f, 0.4f); // Humanoid scale
        }
    }

    void Character::BeginPlay() {
        Actor::BeginPlay();
        if (m_MovementComponent) {
            m_MovementComponent->MaxSpeed = WalkSpeed;
        }
        LOG_INFO("Character '{}' initialized with Health={}/{}", GetName(), Health, MaxHealth);
    }

    void Character::Tick(float deltaTime) {
        Actor::Tick(deltaTime);

        // Ground check based on vertical velocity
        if (auto phys = GetComponent<PhysicsComponent>()) {
            bIsGrounded = (std::abs(phys->LinearVelocity.y) < 0.05f);
        }
    }

    void Character::AddMovementInput(const glm::vec3& direction, float scale) {
        if (m_MovementComponent && scale != 0.0f) {
            glm::vec3 moveDir = glm::length(direction) > 0.001f ? glm::normalize(direction) : direction;
            m_MovementComponent->AddInputVector(moveDir * scale);
        }
    }

    void Character::Jump() {
        if (bIsGrounded) {
            if (auto phys = GetComponent<PhysicsComponent>()) {
                phys->LinearVelocity.y = JumpVelocity;
                bIsGrounded = false;
                LOG_INFO("Character '{}' jumped with impulse {}", GetName(), JumpVelocity);
            }
        }
    }

    ENGINE_CLASS_BEGIN(Character, Actor)
        ENGINE_PROPERTY(Character, WalkSpeed, Float, "Character Movement", "Base walking speed units per second")
        ENGINE_PROPERTY(Character, JumpVelocity, Float, "Character Movement", "Instantaneous vertical jump impulse")
        ENGINE_PROPERTY(Character, bIsGrounded, Bool, "Character Movement", "Current grounded status")
        ENGINE_PROPERTY(Character, Health, Float, "Character Stats", "Current character health points")
        ENGINE_PROPERTY(Character, MaxHealth, Float, "Character Stats", "Maximum character health pool")
    ENGINE_CLASS_END()

} // namespace Engine
