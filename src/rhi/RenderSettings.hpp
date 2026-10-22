/**
 * @file RenderSettings.hpp
 * @brief Global engine render quality, anti-aliasing, and lighting settings.
 */

#pragma once

#include "core/Types.hpp"
#include <string>

namespace Engine {

    /**
     * @brief Anti-aliasing modes supported by the engine renderer.
     */
    enum class AntiAliasingMode : uint8 {
        None = 0,
        FXAA = 1,      ///< Fast Approximate Anti-Aliasing
        MSAA_2x = 2,   ///< Multi-Sample Anti-Aliasing 2x
        MSAA_4x = 3    ///< Multi-Sample Anti-Aliasing 4x
    };

    /**
     * @brief Render quality presets for graphics fidelity and performance.
     */
    enum class RenderQualityPreset : uint8 {
        Low = 0,
        Medium = 1,
        High = 2,
        Ultra = 3,
        Cinematic = 4
    };

    /**
     * @brief Lighting calculation models.
     */
    enum class LightingModel : uint8 {
        PBR_Filmic = 0,       ///< Physically Based Rendering with Cook-Torrance GGX and ACES tone mapping
        Advanced_Phong = 1,   ///< Blinn-Phong specular with hemispheric ambient irradiance
        Soft_Irradiance = 2   ///< Soft omnidirectional shadow and ambient occlusion simulation
    };

    /**
     * @brief Tone mapping operators.
     */
    enum class ToneMappingMode : uint8 {
        ACES_Filmic = 0,
        Reinhard = 1,
        Neutral = 2,
        AgX = 3
    };

    /**
     * @struct RenderSettings
     * @brief Global graphics configuration parameters.
     */
    struct RenderSettings {
        AntiAliasingMode AAMode{AntiAliasingMode::FXAA};
        RenderQualityPreset QualityPreset{RenderQualityPreset::High};
        LightingModel Lighting{LightingModel::PBR_Filmic};
        ToneMappingMode ToneMapping{ToneMappingMode::ACES_Filmic};

        float ResolutionScale{1.0f};          ///< Viewport render scale [0.5 - 2.0]
        bool bEnableShadows{true};            ///< Dynamic shadows flag
        uint32 ShadowResolution{2048};        ///< Shadow map texture resolution
        bool bEnableAmbientOcclusion{true};   ///< Screen-space ambient occlusion approximation
        float AmbientOcclusionStrength{0.6f}; ///< AO intensity
        bool bEnableBloom{true};              ///< HDR bloom and glowing highlights
        float BloomThreshold{0.8f};           ///< Luminance threshold for bloom
        float BloomIntensity{0.4f};           ///< Bloom blend factor
        float Exposure{1.0f};                 ///< Camera exposure compensation
        float Gamma{2.2f};                    ///< Display gamma correction
        bool bPostProcessShadersEnabled{true};///< Custom GLSL shader volume evaluation

        /**
         * @brief Returns global singleton instance of RenderSettings.
         */
        static RenderSettings& Get() {
            static RenderSettings s_Instance;
            return s_Instance;
        }

        /**
         * @brief Applies a quality preset with balanced performance configurations.
         * @param preset Target quality preset.
         */
        void ApplyPreset(RenderQualityPreset preset) {
            QualityPreset = preset;
            switch (preset) {
                case RenderQualityPreset::Low:
                    AAMode = AntiAliasingMode::None;
                    ResolutionScale = 0.75f;
                    bEnableShadows = false;
                    bEnableAmbientOcclusion = false;
                    bEnableBloom = false;
                    ShadowResolution = 1024;
                    break;
                case RenderQualityPreset::Medium:
                    AAMode = AntiAliasingMode::FXAA;
                    ResolutionScale = 1.0f;
                    bEnableShadows = true;
                    bEnableAmbientOcclusion = false;
                    bEnableBloom = true;
                    ShadowResolution = 1024;
                    break;
                case RenderQualityPreset::High:
                    AAMode = AntiAliasingMode::FXAA;
                    ResolutionScale = 1.0f;
                    bEnableShadows = true;
                    bEnableAmbientOcclusion = true;
                    bEnableBloom = true;
                    ShadowResolution = 2048;
                    break;
                case RenderQualityPreset::Ultra:
                    AAMode = AntiAliasingMode::MSAA_2x;
                    ResolutionScale = 1.25f;
                    bEnableShadows = true;
                    bEnableAmbientOcclusion = true;
                    bEnableBloom = true;
                    ShadowResolution = 4096;
                    break;
                case RenderQualityPreset::Cinematic:
                    AAMode = AntiAliasingMode::MSAA_4x;
                    ResolutionScale = 1.5f;
                    bEnableShadows = true;
                    bEnableAmbientOcclusion = true;
                    bEnableBloom = true;
                    ShadowResolution = 4096;
                    break;
            }
        }
    };

} // namespace Engine
