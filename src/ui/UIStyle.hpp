/**
 * @file UIStyle.hpp
 * @brief CSS style properties and declaration parser for game UI.
 */

#pragma once

#include "core/Math.hpp"
#include "core/Types.hpp"
#include <string>
#include <sstream>
#include <algorithm>

namespace Engine {

    enum class UIDisplay : uint8 {
        Block = 0,
        Flex = 1
    };

    enum class UIFlexDirection : uint8 {
        Row = 0,
        Column = 1
    };

    /**
     * @struct UIStyle
     * @brief Computable styling attributes parsed from CSS declarations.
     */
    struct UIStyle {
        glm::vec4 BackgroundColor{0.0f};           ///< RGBA background fill.
        glm::vec4 TextColor{1.0f, 1.0f, 1.0f, 1.0f}; ///< Text font color.
        glm::vec4 BorderColor{0.0f};               ///< Border outline color.
        float BorderWidth{0.0f};
        float BorderRadius{0.0f};

        float PaddingTop{4.0f};
        float PaddingRight{8.0f};
        float PaddingBottom{4.0f};
        float PaddingLeft{8.0f};

        float MarginTop{2.0f};
        float MarginBottom{2.0f};

        float Width{0.0f};                         ///< 0.0f = auto / full width
        float Height{0.0f};                        ///< 0.0f = auto / content height

        UIDisplay Display{UIDisplay::Block};
        UIFlexDirection FlexDirection{UIFlexDirection::Column};

        /**
         * @brief Parses an inline CSS string (e.g., "background-color: #223344; padding: 10px; border-radius: 5px;").
         * @param cssText Semicolon-delimited CSS properties.
         */
        void ParseInline(const std::string& cssText) {
            std::stringstream ss(cssText);
            std::string item;
            while (std::getline(ss, item, ';')) {
                auto colon = item.find(':');
                if (colon == std::string::npos) continue;

                std::string key = item.substr(0, colon);
                std::string val = item.substr(colon + 1);

                auto trim = [](std::string& s) {
                    s.erase(0, s.find_first_not_of(" \t\r\n"));
                    s.erase(s.find_last_not_of(" \t\r\n") + 1);
                };
                trim(key);
                trim(val);

                if (key == "background-color" || key == "background") {
                    BackgroundColor = ParseColor(val);
                } else if (key == "color") {
                    TextColor = ParseColor(val);
                } else if (key == "border-color") {
                    BorderColor = ParseColor(val);
                } else if (key == "border-radius") {
                    BorderRadius = std::stof(val);
                } else if (key == "border-width") {
                    BorderWidth = std::stof(val);
                } else if (key == "padding") {
                    float p = std::stof(val);
                    PaddingTop = PaddingRight = PaddingBottom = PaddingLeft = p;
                } else if (key == "width") {
                    Width = std::stof(val);
                } else if (key == "height") {
                    Height = std::stof(val);
                } else if (key == "display") {
                    if (val == "flex") Display = UIDisplay::Flex;
                    else Display = UIDisplay::Block;
                } else if (key == "flex-direction") {
                    if (val == "row") FlexDirection = UIFlexDirection::Row;
                    else FlexDirection = UIFlexDirection::Column;
                }
            }
        }

    private:
        static glm::vec4 ParseColor(const std::string& str) {
            if (str.starts_with("#")) {
                uint32 hex = static_cast<uint32>(std::stoul(str.substr(1), nullptr, 16));
                if (str.length() == 7) { // #RRGGBB
                    float r = ((hex >> 16) & 0xFF) / 255.0f;
                    float g = ((hex >> 8) & 0xFF) / 255.0f;
                    float b = (hex & 0xFF) / 255.0f;
                    return glm::vec4(r, g, b, 1.0f);
                } else if (str.length() == 9) { // #RRGGBBAA
                    float r = ((hex >> 24) & 0xFF) / 255.0f;
                    float g = ((hex >> 16) & 0xFF) / 255.0f;
                    float b = ((hex >> 8) & 0xFF) / 255.0f;
                    float a = (hex & 0xFF) / 255.0f;
                    return glm::vec4(r, g, b, a);
                }
            }
            if (str == "white") return glm::vec4(1.0f);
            if (str == "black") return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            if (str == "red")   return glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
            if (str == "green") return glm::vec4(0.2f, 0.8f, 0.3f, 1.0f);
            if (str == "blue")  return glm::vec4(0.2f, 0.5f, 0.9f, 1.0f);
            return glm::vec4(0.2f, 0.2f, 0.24f, 1.0f);
        }
    };

} // namespace Engine
