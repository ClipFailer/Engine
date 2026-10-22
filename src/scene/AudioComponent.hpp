/**
 * @file AudioComponent.hpp
 * @brief Component for playing spatial 3D audio cues or 2D audio tracks.
 */

#pragma once

#include "Component.hpp"
#include <string>

namespace Engine {

    /**
     * @class AudioComponent
     * @brief Audio playback component with 3D spatial attenuation.
     */
    class AudioComponent : public Component {
        GENERATED_BODY()
    public:
        AudioComponent();

        std::string SoundFilePath{"assets/sounds/test.wav"}; ///< Sound file asset path.
        float Volume{1.0f};                                  ///< Playback volume [0..1].
        float Pitch{1.0f};                                   ///< Playback pitch multiplier.
        bool bLoop{false};                                   ///< Repeat sound on finish.
        bool bPlayOnStart{false};                            ///< Auto play when simulation starts.
        bool bIs3D{true};                                    ///< Spatial 3D sound attenuation.
        float MinDistance{1.0f};                             ///< Full volume radius.
        float MaxDistance{25.0f};                            ///< Audibility cutoff radius.
    };

} // namespace Engine
