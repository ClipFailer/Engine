/**
 * @file Actor.cpp
 * @brief Implementation and reflection metadata for Actor.
 */

#include "Actor.hpp"
#include <algorithm>

namespace Engine {

    Actor::Actor() : Object("Actor") {}

    Actor::Actor(std::string name) : Object(std::move(name)) {}

    void Actor::BeginPlay() {
        for (auto& comp : m_Components) {
            if (comp->bIsActive) {
                comp->BeginPlay();
            }
        }
    }

    void Actor::Tick(float deltaTime) {
        for (auto& comp : m_Components) {
            if (comp->bIsActive) {
                comp->Tick(deltaTime);
            }
        }
    }

    void Actor::EndPlay() {
        for (auto& comp : m_Components) {
            if (comp->bIsActive) {
                comp->EndPlay();
            }
        }
    }

    void Actor::RemoveComponent(Component* comp) {
        auto it = std::remove_if(m_Components.begin(), m_Components.end(),
            [comp](const std::unique_ptr<Component>& ptr) {
                return ptr.get() == comp;
            });
        m_Components.erase(it, m_Components.end());
    }

    ENGINE_CLASS_BEGIN(Actor, Object)
        ENGINE_PROPERTY(Actor, Transform, Transform, "Transform", "Spatial transform (Location, Rotation, Scale)")
        ENGINE_PROPERTY(Actor, bIsVisible, Bool, "Rendering", "Visibility flag in scene")
    ENGINE_CLASS_END()

} // namespace Engine
