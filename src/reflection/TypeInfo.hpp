/**
 * @file TypeInfo.hpp
 * @brief Reflection type identifiers and kind enumerations.
 */

#pragma once

#include "core/Types.hpp"

namespace Engine {

    /**
     * @brief Fundamental reflected property types for serialization and editor inspection.
     */
    enum class PropertyType : uint8 {
        Float,
        Int,
        Bool,
        String,
        Vec2,
        Vec3,
        Vec4,
        Color3,
        Color4,
        Transform,
        Enum
    };

    /**
     * @brief Converts PropertyType enum to readable string view.
     * @param type The property type.
     * @return String view name.
     */
    constexpr std::string_view PropertyTypeToString(PropertyType type) {
        switch (type) {
            case PropertyType::Float:     return "Float";
            case PropertyType::Int:       return "Int";
            case PropertyType::Bool:      return "Bool";
            case PropertyType::String:    return "String";
            case PropertyType::Vec2:      return "Vector2";
            case PropertyType::Vec3:      return "Vector3";
            case PropertyType::Vec4:      return "Vector4";
            case PropertyType::Color3:    return "Color3";
            case PropertyType::Color4:    return "Color4";
            case PropertyType::Transform: return "Transform";
            case PropertyType::Enum:      return "Enum";
        }
        return "Unknown";
    }

} // namespace Engine
