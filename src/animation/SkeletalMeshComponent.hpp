/**
 * @file SkeletalMeshComponent.hpp
 * @brief Component managing skeletal hierarchies, skinning matrices, and animation playback.
 */

#pragma once

#include "scene/Component.hpp"
#include "Skeleton.hpp"
#include "AnimationSequence.hpp"
#include <memory>

namespace Engine {

    /**
     * @class SkeletalMeshComponent
     * @brief Animates skeletal mesh joints along keyframed animation tracks.
     */
    class SkeletalMeshComponent : public Component {
        GENERATED_BODY()
    public:
        SkeletalMeshComponent();
        ~SkeletalMeshComponent() override = default;

        void Tick(float deltaTime) override;

        /**
         * @brief Plays an animation sequence.
         * @param anim Shared pointer to AnimationSequence.
         * @param bLooping Loop playback flag.
         */
        void PlayAnimation(std::shared_ptr<AnimationSequence> anim, bool bLooping = true);

        /**
         * @brief Pauses active animation playback.
         */
        void Pause() { bIsPlaying = false; }

        /**
         * @brief Resumes animation playback.
         */
        void Resume() { bIsPlaying = true; }

        Skeleton MeshSkeleton;
        std::shared_ptr<AnimationSequence> ActiveAnimation;

        float PlaybackSpeed{1.0f};
        bool bLoop{true};
        bool bIsPlaying{true};
        float CurrentTime{0.0f};
    };

} // namespace Engine
