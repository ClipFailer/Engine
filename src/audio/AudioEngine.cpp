/**
 * @file AudioEngine.cpp
 * @brief Implementation of AudioEngine using miniaudio.
 */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "AudioEngine.hpp"
#include "core/Log.hpp"

namespace Engine {

    AudioEngine::AudioEngine() = default;

    AudioEngine::~AudioEngine() {
        Shutdown();
    }

    bool AudioEngine::Initialize() {
        if (m_bInitialized) return true;

        m_Engine = std::make_unique<ma_engine>();
        ma_result result = ma_engine_init(nullptr, m_Engine.get());
        if (result != MA_SUCCESS) {
            LOG_WARN("Could not initialize audio device (error {}). Audio playback will be disabled.", static_cast<int>(result));
            m_Engine.reset();
            return false;
        }

        m_bInitialized = true;
        LOG_INFO("AudioEngine successfully initialized with miniaudio");
        return true;
    }

    void AudioEngine::Shutdown() {
        if (m_bInitialized && m_Engine) {
            ma_engine_uninit(m_Engine.get());
            m_Engine.reset();
            m_bInitialized = false;
            LOG_INFO("AudioEngine shut down cleanly");
        }
    }

    void AudioEngine::PlaySound2D(const std::string& filePath, float volume, float pitch) {
        if (!m_bInitialized || !m_Engine) return;

        ma_engine_play_sound(m_Engine.get(), filePath.c_str(), nullptr);
    }

    void AudioEngine::PlaySound3D(const std::string& filePath, const glm::vec3& position, float volume, float pitch) {
        if (!m_bInitialized || !m_Engine) return;

        ma_sound sound;
        ma_result result = ma_sound_init_from_file(m_Engine.get(), filePath.c_str(), 0, nullptr, nullptr, &sound);
        if (result == MA_SUCCESS) {
            ma_sound_set_position(&sound, position.x, position.y, position.z);
            ma_sound_set_volume(&sound, volume);
            ma_sound_set_pitch(&sound, pitch);
            ma_sound_set_spatialization_enabled(&sound, MA_TRUE);
            ma_sound_start(&sound);
        }
    }

    void AudioEngine::UpdateListener(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up) {
        if (!m_bInitialized || !m_Engine) return;

        ma_engine_listener_set_position(m_Engine.get(), 0, position.x, position.y, position.z);
        ma_engine_listener_set_direction(m_Engine.get(), 0, forward.x, forward.y, forward.z);
        ma_engine_listener_set_world_up(m_Engine.get(), 0, up.x, up.y, up.z);
    }

} // namespace Engine
