/**
 * @file PhysicsComponent.cpp
 * @brief Implementation and reflection metadata for PhysicsComponent.
 */

#include "PhysicsComponent.hpp"

namespace Engine {

    PhysicsComponent::PhysicsComponent() : Component("PhysicsComponent") {}

    ENGINE_CLASS_BEGIN(PhysicsComponent, Component)
        ENGINE_PROPERTY(PhysicsComponent, Mass, Float, "Physics", "Mass in kg")
        ENGINE_PROPERTY(PhysicsComponent, bUseGravity, Bool, "Physics", "Apply global gravity")
        ENGINE_PROPERTY(PhysicsComponent, bIsStatic, Bool, "Physics", "Static immovable body")
        ENGINE_PROPERTY(PhysicsComponent, LinearVelocity, Vec3, "Physics", "Current linear velocity")
        ENGINE_PROPERTY(PhysicsComponent, LinearDamping, Float, "Physics", "Linear air drag")
    ENGINE_CLASS_END()

} // namespace Engine
