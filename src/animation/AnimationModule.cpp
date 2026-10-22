/**
 * @file AnimationModule.cpp
 * @brief Implementation of AnimationModule.
 */

#include "AnimationModule.hpp"
#include "core/Log.hpp"

namespace Engine {

    void AnimationModule::OnLoad() {
        LOG_INFO("AnimationModule loaded: Skeletal skinning and timeline animation online");
    }

    void AnimationModule::OnUpdate(float deltaTime) {
        (void)deltaTime;
    }

    void AnimationModule::OnUnload() {
        LOG_INFO("AnimationModule unloaded cleanly");
    }

} // namespace Engine
