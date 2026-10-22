/**
 * @file Property.hpp
 * @brief Property descriptor representing a reflected member field.
 */

#pragma once

#include "TypeInfo.hpp"
#include "core/Math.hpp"
#include <string>
#include <cstddef>
#include <functional>

namespace Engine {

    /**
     * @class PropertyDescriptor
     * @brief Encapsulates runtime metadata and memory offsets for an exposed class field.
     */
    class PropertyDescriptor {
    public:
        /**
         * @brief Constructs a property descriptor.
         * @param name Name of the field.
         * @param type Data type.
         * @param offset Byte offset in class structure.
         * @param category UI category grouping.
         * @param tooltip Description for tooltip in inspector.
         */
        PropertyDescriptor(std::string name,
                           PropertyType type,
                           std::size_t offset,
                           std::string category = "General",
                           std::string tooltip = "")
            : m_Name(std::move(name)),
              m_Type(type),
              m_Offset(offset),
              m_Category(std::move(category)),
              m_Tooltip(std::move(tooltip)) {}

        [[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
        [[nodiscard]] PropertyType GetType() const noexcept { return m_Type; }
        [[nodiscard]] std::size_t GetOffset() const noexcept { return m_Offset; }
        [[nodiscard]] const std::string& GetCategory() const noexcept { return m_Category; }
        [[nodiscard]] const std::string& GetTooltip() const noexcept { return m_Tooltip; }

        /**
         * @brief Retrieves raw pointer to the member variable within an instance.
         * @param instance Pointer to the class instance.
         * @return Raw void pointer to member.
         */
        [[nodiscard]] void* GetValuePtr(void* instance) const noexcept {
            return static_cast<char*>(instance) + m_Offset;
        }

        /**
         * @brief Retrieves const raw pointer to the member variable within an instance.
         * @param instance Pointer to the class instance.
         * @return Const void pointer to member.
         */
        [[nodiscard]] const void* GetValuePtr(const void* instance) const noexcept {
            return static_cast<const char*>(instance) + m_Offset;
        }

        /**
         * @brief Type-safe typed reference access to member.
         * @tparam T Property type.
         * @param instance Class instance pointer.
         * @return Reference to member variable.
         */
        template<typename T>
        [[nodiscard]] T& GetValue(void* instance) const noexcept {
            return *reinterpret_cast<T*>(GetValuePtr(instance));
        }

        template<typename T>
        [[nodiscard]] const T& GetValue(const void* instance) const noexcept {
            return *reinterpret_cast<const T*>(GetValuePtr(instance));
        }

    private:
        std::string m_Name;
        PropertyType m_Type;
        std::size_t m_Offset;
        std::string m_Category;
        std::string m_Tooltip;
    };

} // namespace Engine
