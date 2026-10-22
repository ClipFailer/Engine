/**
 * @file CustomComponent.cpp
 * @brief Implementation of CustomComponent and reflection properties.
 */

#include "CustomComponent.hpp"
#include "core/Log.hpp"

namespace Engine {

    CustomComponent::CustomComponent() : Component("CustomComponent") {}

    CustomComponent::CustomComponent(std::string typeName)
        : Component(typeName), ComponentType(typeName) {}

    void CustomComponent::BeginPlay() {
        Component::BeginPlay();
        LOG_INFO("CustomComponent '{}' active!", ComponentType);
    }

    void CustomComponent::Tick(float deltaTime) {
        Component::Tick(deltaTime);
    }

    ENGINE_CLASS_BEGIN(CustomComponent, Component)
        ENGINE_PROPERTY(CustomComponent, ComponentType, String, "Custom Settings", "User component type identifier")
        ENGINE_PROPERTY(CustomComponent, CustomScalar, Float, "Custom Settings", "Scalar gameplay property")
        ENGINE_PROPERTY(CustomComponent, bIsEnabled, Bool, "Custom Settings", "Active toggle for component")
        ENGINE_PROPERTY(CustomComponent, CustomTag, String, "Custom Settings", "Custom tag value")
    ENGINE_CLASS_END()

} // namespace Engine
