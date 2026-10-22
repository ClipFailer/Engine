/**
 * @file ShaderActor.hpp
 * @brief Invisible scene entity hosting a custom GLSL post-process trigger volume.
 */

#pragma once

#include "Actor.hpp"
#include "ShaderComponent.hpp"

namespace Engine {

    /**
     * @class ShaderActor
     * @brief An invisible level actor providing a custom post-process shader trigger volume.
     */
    class ShaderActor : public Actor {
        GENERATED_BODY()
    public:
        ShaderActor();
        explicit ShaderActor(std::string name);
        ~ShaderActor() override = default;

        [[nodiscard]] ShaderComponent* GetShaderComponent() const noexcept {
            return GetComponent<ShaderComponent>();
        }
    };

} // namespace Engine
