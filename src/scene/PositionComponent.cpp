/**
 * @file PositionComponent.cpp
 * @brief Implementation and reflection metadata for PositionComponent.
 */

#include "PositionComponent.hpp"

namespace Engine {

    PositionComponent::PositionComponent()
        : Component("PositionComponent") {}

    PositionComponent::PositionComponent(const glm::vec3& position)
        : Component("PositionComponent"), Transform(position) {}

    ENGINE_CLASS_BEGIN(PositionComponent, Component)
        ENGINE_PROPERTY(PositionComponent, Transform, Transform, "Transform", "World position, rotation, and scale")
    ENGINE_CLASS_END()

} // namespace Engine
