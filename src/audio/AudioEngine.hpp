/**
 * @file AudioEngine.hpp
 * @brief High-performance audio subsystem based on miniaudio supporting 2D/3D spatial playback.
 */

#pragma once

#include "core/Math.hpp"
#include "core/Types.hpp"
#include <string>
#include <memory>

struct ma_engine;

namespace Engine {

    /**
     * @class AudioEngine
     * @brief Audio playback engine providing 2D cues and 3D spatial sound with listener orientation.
     */
    class AudioEngine {
    public:
        AudioEngine();
        ~AudioEngine();

        AudioEngine(const AudioEngine&) = delete;
        AudioEngine& operator=(const AudioEngine&) = delete;

        /**
         * @brief Initializes audio device and engine context.
         * @return True if initialized successfully.
         */
        bool Initialize();

        /**
         * @brief Shuts down the audio context and releases device.
         */
        void Shutdown();

        /**
         * @brief Plays a 2D sound file.
         * @param filePath Path to sound file.
         * @param volume Volume level [0..1].
         * @param pitch Pitch multiplier.
         */
        void PlaySound2D(const std::string& filePath, float volume = 1.0f, float pitch = 1.0f);

        /**
         * @brief Plays a 3D spatialized sound at a world position.
         * @param filePath Path to audio file.
         * @param position World space position of audio emitter.
         * @param volume Volume level [0..1].
         * @param pitch Pitch multiplier.
         */
        void PlaySound3D(const std::string& filePath, const glm::vec3& position, float volume = 1.0f, float pitch = 1.0f);

        /**
         * @brief Updates listener position and orientation from camera.
         * @param position Camera location.
         * @param forward Camera forward unit vector.
         * @param up Camera up unit vector.
         */
        void UpdateListener(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);

    private:
        std::unique_ptr<ma_engine> m_Engine;
        bool m_bInitialized{false};
    };

} // namespace Engine
