/**
 * @file ShaderComponent.hpp
 * @brief Component containing custom GLSL shader logic, trigger zone activation, and global toggle.
 */

#pragma once

#include "Component.hpp"
#include "core/Math.hpp"
#include <string>

namespace Engine {

    /**
     * @brief Post-process visual effect type produced by this shader volume.
     */
    enum class ShaderEffectType : uint8 {
        None = 0,
        Vignette = 1,
        NightVision = 2,
        Grayscale = 3,
        Sepia = 4,
        Invert = 5,
        Custom = 6
    };

    /**
     * @class ShaderComponent
     * @brief Holds a programmable GLSL post-process shader, trigger volume boundary, and activeEverywhere mode.
     */
    class ShaderComponent : public Component {
        GENERATED_BODY()
    public:
        ShaderComponent();
        ~ShaderComponent() override = default;

        std::string ShaderName{"CustomScreenShader"};
        std::string ShaderCode;               ///< Editable GLSL fragment/post-process code
        ShaderEffectType EffectType{ShaderEffectType::Vignette}; ///< Active post-process effect preset
        float EffectIntensity{1.0f};          ///< Visual effect blend intensity [0.0 - 2.0]
        bool bActiveEverywhere{false};        ///< Toggle: if true, shader affects the whole scene everywhere
        bool bIsSphereZone{false};            ///< True for spherical trigger zone, false for box (AABB)
        glm::vec3 TriggerHalfExtents{5.0f};   ///< Box trigger zone extents
        glm::vec3 TriggerOffset{0.0f};        ///< Center offset from parent Actor
        float TriggerRadius{5.0f};            ///< Sphere trigger zone radius

        /**
         * @brief Checks if camera is inside the trigger zone or if activeEverywhere is enabled.
         * @param cameraPos World coordinates of camera.
         * @param actorPos World coordinates of parent actor.
         * @return True if shader should be active.
         */
        [[nodiscard]] bool IsActiveAtPosition(const glm::vec3& cameraPos, const glm::vec3& actorPos) const noexcept;

        /**
         * @brief Resolves active effect type from enum or shader source pattern.
         */
        [[nodiscard]] ShaderEffectType GetResolvedEffectType() const;

        /**
         * @brief Applies a preset GLSL shader program and updates EffectType.
         * @param presetName Name of preset ("Vignette", "NightVision", "Grayscale", "Sepia", "Invert").
         */
        void LoadPreset(const std::string& presetName);

        [[nodiscard]] static std::string GetDefaultShaderCode();
    };

} // namespace Engine
