/**
 * @file AnimationSequence.hpp
 * @brief Skeletal animation keyframes, bone tracks, and timeline interpolation.
 */

#pragma once

#include "core/Math.hpp"
#include <string>
#include <vector>
#include <algorithm>

namespace Engine {

    struct VectorKey {
        float Time{0.0f};
        glm::vec3 Value{0.0f};
    };

    struct QuatKey {
        float Time{0.0f};
        glm::quat Value{1.0f, 0.0f, 0.0f, 0.0f};
    };

    /**
     * @struct BoneTrack
     * @brief Animated curves for translation, rotation, and scale of a specific bone.
     */
    struct BoneTrack {
        std::string BoneName;
        std::vector<VectorKey> Positions;
        std::vector<QuatKey> Rotations;
        std::vector<VectorKey> Scales;

        glm::vec3 EvaluatePosition(float time) const {
            if (Positions.empty()) return glm::vec3(0.0f);
            if (Positions.size() == 1 || time <= Positions.front().Time) return Positions.front().Value;
            if (time >= Positions.back().Time) return Positions.back().Value;

            for (size_t i = 0; i < Positions.size() - 1; ++i) {
                if (time >= Positions[i].Time && time <= Positions[i + 1].Time) {
                    float factor = (time - Positions[i].Time) / (Positions[i + 1].Time - Positions[i].Time);
                    return glm::mix(Positions[i].Value, Positions[i + 1].Value, factor);
                }
            }
            return Positions.back().Value;
        }

        glm::quat EvaluateRotation(float time) const {
            if (Rotations.empty()) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            if (Rotations.size() == 1 || time <= Rotations.front().Time) return Rotations.front().Value;
            if (time >= Rotations.back().Time) return Rotations.back().Value;

            for (size_t i = 0; i < Rotations.size() - 1; ++i) {
                if (time >= Rotations[i].Time && time <= Rotations[i + 1].Time) {
                    float factor = (time - Rotations[i].Time) / (Rotations[i + 1].Time - Rotations[i].Time);
                    return glm::slerp(Rotations[i].Value, Rotations[i + 1].Value, factor);
                }
            }
            return Rotations.back().Value;
        }

        glm::vec3 EvaluateScale(float time) const {
            if (Scales.empty()) return glm::vec3(1.0f);
            if (Scales.size() == 1 || time <= Scales.front().Time) return Scales.front().Value;
            if (time >= Scales.back().Time) return Scales.back().Value;

            for (size_t i = 0; i < Scales.size() - 1; ++i) {
                if (time >= Scales[i].Time && time <= Scales[i + 1].Time) {
                    float factor = (time - Scales[i].Time) / (Scales[i + 1].Time - Scales[i].Time);
                    return glm::mix(Scales[i].Value, Scales[i + 1].Value, factor);
                }
            }
            return Scales.back().Value;
        }
    };

    /**
     * @class AnimationSequence
     * @brief Keyframed skeletal animation clip spanning multiple bone channels.
     */
    class AnimationSequence {
    public:
        AnimationSequence(std::string name = "DefaultAnim", float duration = 1.0f, float ticksPerSec = 30.0f)
            : m_Name(std::move(name)), m_Duration(duration), m_TicksPerSecond(ticksPerSec) {}

        [[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
        [[nodiscard]] float GetDuration() const noexcept { return m_Duration; }
        [[nodiscard]] float GetTicksPerSecond() const noexcept { return m_TicksPerSecond; }
        [[nodiscard]] const std::vector<BoneTrack>& GetTracks() const noexcept { return m_Tracks; }

        void AddTrack(BoneTrack track) {
            m_Tracks.push_back(std::move(track));
        }

        [[nodiscard]] const BoneTrack* FindTrack(const std::string& boneName) const {
            for (const auto& track : m_Tracks) {
                if (track.BoneName == boneName) return &track;
            }
            return nullptr;
        }

    private:
        std::string m_Name;
        float m_Duration{1.0f};
        float m_TicksPerSecond{30.0f};
        std::vector<BoneTrack> m_Tracks;
    };

} // namespace Engine
