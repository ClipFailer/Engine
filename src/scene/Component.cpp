/**
 * @file Component.cpp
 * @brief Implementation and reflection registration of Component class.
 */

#include "Component.hpp"

namespace Engine {

    Component::Component() : Object("Component") {}

    Component::Component(std::string name) : Object(std::move(name)) {}

    ENGINE_CLASS_BEGIN(Component, Object)
        ENGINE_PROPERTY(Component, bIsActive, Bool, "General", "Whether this component is enabled")
    ENGINE_CLASS_END()

} // namespace Engine
