/**
 * @file AnimationModule.hpp
 * @brief Skeletal animation module implementation.
 */

#pragma once

#include "module/IModule.hpp"

namespace Engine {

    /**
     * @class AnimationModule
     * @brief Engine module managing skeletal skinning, clips, and animation blending.
     */
    class AnimationModule : public IModule {
    public:
        AnimationModule() = default;
        ~AnimationModule() override = default;

        void OnLoad() override;
        void OnUpdate(float deltaTime) override;
        void OnUnload() override;

        [[nodiscard]] std::string_view GetName() const override { return "AnimationModule"; }
    };

} // namespace Engine
