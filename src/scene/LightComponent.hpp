/**
 * @file LightComponent.hpp
 * @brief Lighting component providing directional or point illumination.
 */

#pragma once

#include "Component.hpp"
#include "core/Math.hpp"

namespace Engine {

    enum class LightType : uint8 {
        Directional = 0,
        Point = 1
    };

    /**
     * @class LightComponent
     * @brief Emits directional or point lighting into the scene.
     */
    class LightComponent : public Component {
        GENERATED_BODY()
    public:
        LightComponent();
        explicit LightComponent(LightType type);

        LightType Type{LightType::Directional};
        glm::vec3 LightColor{1.0f, 0.95f, 0.88f}; ///< Light emission RGB.
        float Intensity{1.0f};                    ///< Brightness multiplier.
        float Radius{15.0f};                      ///< Attenuation radius for point light.
    };

} // namespace Engine
