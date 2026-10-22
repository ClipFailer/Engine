/**
 * @file SkeletalMeshComponent.cpp
 * @brief Implementation and reflection metadata for SkeletalMeshComponent.
 */

#include "SkeletalMeshComponent.hpp"
#include <cmath>

namespace Engine {

    SkeletalMeshComponent::SkeletalMeshComponent()
        : Component("SkeletalMeshComponent") {
        // Setup a default 2-bone armature (Root -> Arm)
        MeshSkeleton.AddBone("Root", -1, Transform(glm::vec3(0.0f, 0.0f, 0.0f)));
        MeshSkeleton.AddBone("Arm",   0, Transform(glm::vec3(0.0f, 1.0f, 0.0f)));
        MeshSkeleton.UpdateHierarchy();
    }

    void SkeletalMeshComponent::PlayAnimation(std::shared_ptr<AnimationSequence> anim, bool bLooping) {
        ActiveAnimation = std::move(anim);
        bLoop = bLooping;
        CurrentTime = 0.0f;
        bIsPlaying = true;
    }

    void SkeletalMeshComponent::Tick(float deltaTime) {
        if (!bIsPlaying || !ActiveAnimation) return;

        CurrentTime += deltaTime * PlaybackSpeed;
        float duration = ActiveAnimation->GetDuration();

        if (CurrentTime > duration) {
            if (bLoop && duration > 0.0f) {
                CurrentTime = std::fmod(CurrentTime, duration);
            } else {
                CurrentTime = duration;
                bIsPlaying = false;
            }
        }

        // Evaluate bone tracks
        auto& bones = MeshSkeleton.GetBones();
        for (auto& bone : bones) {
            if (const BoneTrack* track = ActiveAnimation->FindTrack(bone.Name)) {
                bone.LocalTransform.Position = track->EvaluatePosition(CurrentTime);
                bone.LocalTransform.Rotation = glm::degrees(glm::eulerAngles(track->EvaluateRotation(CurrentTime)));
                bone.LocalTransform.Scale    = track->EvaluateScale(CurrentTime);
            }
        }

        MeshSkeleton.UpdateHierarchy();
    }

    ENGINE_CLASS_BEGIN(SkeletalMeshComponent, Component)
        ENGINE_PROPERTY(SkeletalMeshComponent, PlaybackSpeed, Float, "Animation", "Playback speed multiplier")
        ENGINE_PROPERTY(SkeletalMeshComponent, bLoop, Bool, "Animation", "Loop animation sequence")
        ENGINE_PROPERTY(SkeletalMeshComponent, bIsPlaying, Bool, "Animation", "Active playback state")
        ENGINE_PROPERTY(SkeletalMeshComponent, CurrentTime, Float, "Animation", "Current timeline time in seconds")
    ENGINE_CLASS_END()

} // namespace Engine
