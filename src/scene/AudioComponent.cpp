/**
 * @file AudioComponent.cpp
 * @brief Implementation and reflection metadata for AudioComponent.
 */

#include "AudioComponent.hpp"

namespace Engine {

    AudioComponent::AudioComponent() : Component("AudioComponent") {}

    ENGINE_CLASS_BEGIN(AudioComponent, Component)
        ENGINE_PROPERTY(AudioComponent, SoundFilePath, String, "Audio", "Path to wave audio file")
        ENGINE_PROPERTY(AudioComponent, Volume, Float, "Audio", "Playback volume")
        ENGINE_PROPERTY(AudioComponent, Pitch, Float, "Audio", "Playback pitch shift")
        ENGINE_PROPERTY(AudioComponent, bLoop, Bool, "Audio", "Loop playback endlessly")
        ENGINE_PROPERTY(AudioComponent, bPlayOnStart, Bool, "Audio", "Play automatically on simulation begin")
        ENGINE_PROPERTY(AudioComponent, bIs3D, Bool, "Audio", "Enable 3D spatial attenuation")
    ENGINE_CLASS_END()

} // namespace Engine
