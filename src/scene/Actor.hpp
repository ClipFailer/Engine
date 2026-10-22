/**
 * @file Actor.hpp
 * @brief Base Actor class in the Unreal-style scene entity hierarchy.
 */

#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "core/Math.hpp"
#include <vector>
#include <memory>
#include <string>
#include <type_traits>
#include <nlohmann/json.hpp>

namespace Engine {

    /**
     * @class Actor
     * @brief An entity that can be placed or spawned into a level/world.
     */
    class Actor : public Object {
        GENERATED_BODY()
    public:
        Actor();
        explicit Actor(std::string name, bool bAttachDefaultComponents = true);
        ~Actor() override = default;

        /**
         * @brief Called when level simulation starts.
         */
        virtual void BeginPlay();

        /**
         * @brief Called every frame during active simulation.
         * @param deltaTime Elapsed frame time in seconds.
         */
        virtual void Tick(float deltaTime);

        /**
         * @brief Called when simulation ends.
         */
        virtual void EndPlay();

        /**
         * @brief Adds a new component to this actor.
         * @tparam T Component class type.
         * @param args Constructor arguments.
         * @return Raw pointer to created component.
         */
        template<typename T, typename... Args>
        T* AddComponent(Args&&... args) {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            comp->SetOwner(this);
            T* ptr = comp.get();
            m_Components.push_back(std::move(comp));
            return ptr;
        }

        /**
         * @brief Adds an already instantiated component pointer to this actor.
         * @param comp Unique pointer to component.
         * @return Raw pointer to added component.
         */
        Component* AddComponent(std::unique_ptr<Component> comp) {
            if (!comp) return nullptr;
            comp->SetOwner(this);
            Component* ptr = comp.get();
            m_Components.push_back(std::move(comp));
            return ptr;
        }

        /**
         * @brief Searches for the first component of type T.
         * @tparam T Component class type.
         * @return Pointer to component or nullptr if not found.
         */
        template<typename T>
        [[nodiscard]] T* GetComponent() const noexcept {
            for (const auto& comp : m_Components) {
                if (auto casted = dynamic_cast<T*>(comp.get())) {
                    return casted;
                }
            }
            return nullptr;
        }

        /**
         * @brief Returns collection of all attached components.
         * @return Vector of unique pointers to components.
         */
        [[nodiscard]] const std::vector<std::unique_ptr<Component>>& GetComponents() const noexcept {
            return m_Components;
        }

        /**
         * @brief Removes a component from this actor by pointer.
         * @param comp Pointer to component.
         */
        void RemoveComponent(Component* comp);

        /**
         * @brief Removes all components of type T from this actor.
         * @tparam T Component class type.
         */
        template<typename T>
        void RemoveComponent() {
            auto it = std::remove_if(m_Components.begin(), m_Components.end(),
                [](const std::unique_ptr<Component>& ptr) {
                    return dynamic_cast<T*>(ptr.get()) != nullptr;
                });
            m_Components.erase(it, m_Components.end());
        }

        Transform Transform; ///< Actor local/world spatial transformation.
        std::string Tag{"Default"}; ///< Identification tag (e.g., Player, Enemy, Obstacle).
        std::string AssetPath{}; ///< Filepath to .actor blueprint file on disk.
        bool bIsVisible{true};

        /**
         * @brief Serializes this Actor and all its components to JSON.
         * @param bIncludePosition True to serialize spatial world position (for scenes), false for asset templates.
         * @return JSON representation of Actor.
         */
        [[nodiscard]] nlohmann::json Serialize(bool bIncludePosition = true) const;

        /**
         * @brief Deserializes properties and components from JSON into this Actor.
         * @param j Source JSON data.
         * @param bRestorePosition Whether to restore spatial world position.
         * @return True if deserialization succeeded.
         */
        bool Deserialize(const nlohmann::json& j, bool bRestorePosition = true);

        /**
         * @brief Saves this Actor's definition and component setup to a .actor file.
         * @param filepath Destination path on disk.
         * @return True if saved successfully.
         */
        bool SaveToAssetFile(const std::string& filepath) const;

        /**
         * @brief Loads and applies an Actor setup from a .actor file.
         * @param filepath Source path on disk.
         * @return True if loaded successfully.
         */
        bool LoadFromAssetFile(const std::string& filepath);

    protected:
        std::vector<std::unique_ptr<Component>> m_Components;
    };

} // namespace Engine
