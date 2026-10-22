/**
 * @file Object.hpp
 * @brief Base Object class serving as the root of the Unreal-style engine hierarchy.
 */

#pragma once

#include "core/Types.hpp"
#include "core/UUID.hpp"
#include <string>
#include <string_view>

namespace Engine {

    /**
     * @class Object
     * @brief Root class for all inspectable, serializable, and reflected entities in the engine.
     */
    class Object {
    public:
        /**
         * @brief Default constructor creating a unique UUID.
         */
        Object();

        /**
         * @brief Constructor with explicit name.
         * @param name Display and identification name.
         */
        explicit Object(std::string name);

        /**
         * @brief Virtual destructor for polymorphic destruction.
         */
        virtual ~Object() = default;

        /**
         * @brief Returns the unique identifier for this object.
         * @return 64-bit UUID.
         */
        [[nodiscard]] UUID GetUUID() const noexcept { return m_UUID; }

        /**
         * @brief Returns the human-readable display name.
         * @return Name string.
         */
        [[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }

        /**
         * @brief Sets the object's display name.
         * @param name New name.
         */
        void SetName(std::string name) { m_Name = std::move(name); }

        /**
         * @brief Virtual method returning the reflected class name.
         * @return Class name string view.
         */
        [[nodiscard]] virtual std::string_view GetClassName() const { return "Object"; }

        /**
         * @brief Static method returning the class name.
         * @return Class name string view.
         */
        [[nodiscard]] static std::string_view StaticClassName() { return "Object"; }

    protected:
        UUID m_UUID;
        std::string m_Name{"Object"};
    };

} // namespace Engine
