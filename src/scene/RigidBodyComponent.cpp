/**
 * @file RigidBodyComponent.cpp
 * @brief Implementation and reflection metadata for RigidBodyComponent.
 */

#include "RigidBodyComponent.hpp"

namespace Engine {

    RigidBodyComponent::RigidBodyComponent() : Component("RigidBodyComponent") {}

    ENGINE_CLASS_BEGIN(RigidBodyComponent, Component)
        ENGINE_PROPERTY(RigidBodyComponent, Mass, Float, "Physics", "Body mass in kg")
        ENGINE_PROPERTY(RigidBodyComponent, bUseGravity, Bool, "Physics", "Subject to gravity forces")
        ENGINE_PROPERTY(RigidBodyComponent, bIsStatic, Bool, "Physics", "Immovable static collision body")
        ENGINE_PROPERTY(RigidBodyComponent, Restitution, Float, "Physics", "Coefficient of restitution (bounciness)")
        ENGINE_PROPERTY(RigidBodyComponent, Friction, Float, "Physics", "Surface friction coefficient")
        ENGINE_PROPERTY(RigidBodyComponent, LinearVelocity, Vec3, "Physics", "Current velocity vector")
    ENGINE_CLASS_END()

} // namespace Engine
