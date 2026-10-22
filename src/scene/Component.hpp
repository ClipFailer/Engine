/**
 * @file Component.hpp
 * @brief Base component class attached to Actors in the scene.
 */

#pragma once

#include "Object.hpp"
#include "reflection/Reflection.hpp"

namespace Engine {

    class Actor;

    /**
     * @class Component
     * @brief Base class for modular functionality attached to an Actor.
     */
    class Component : public Object {
        GENERATED_BODY()
    public:
        Component();
        explicit Component(std::string name);
        ~Component() override = default;

        /**
         * @brief Called when simulation starts (Play mode begins).
         */
        virtual void BeginPlay() {}

        /**
         * @brief Called every simulation tick.
         * @param deltaTime Elapsed seconds since last tick.
         */
        virtual void Tick(float deltaTime) {}

        /**
         * @brief Called when simulation stops.
         */
        virtual void EndPlay() {}

        /**
         * @brief Retrieves the owner Actor holding this component.
         * @return Pointer to owner Actor or nullptr.
         */
        [[nodiscard]] Actor* GetOwner() const noexcept { return m_Owner; }

        /**
         * @brief Sets the owning Actor.
         * @param owner Pointer to Actor.
         */
        void SetOwner(Actor* owner) noexcept { m_Owner = owner; }

        bool bIsActive{true};

    protected:
        Actor* m_Owner{nullptr};
    };

} // namespace Engine
