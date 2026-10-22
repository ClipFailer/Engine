/**
 * @file LightComponent.cpp
 * @brief Implementation and reflection metadata for LightComponent.
 */

#include "LightComponent.hpp"

namespace Engine {

    LightComponent::LightComponent() : Component("LightComponent"), Type(LightType::Directional) {}

    LightComponent::LightComponent(LightType type) : Component("LightComponent"), Type(type) {}

    ENGINE_CLASS_BEGIN(LightComponent, Component)
        ENGINE_PROPERTY(LightComponent, LightColor, Color3, "Lighting", "Light emission RGB color")
        ENGINE_PROPERTY(LightComponent, Intensity, Float, "Lighting", "Light brightness multiplier")
        ENGINE_PROPERTY(LightComponent, Radius, Float, "Lighting", "Point light effective radius")
    ENGINE_CLASS_END()

} // namespace Engine
