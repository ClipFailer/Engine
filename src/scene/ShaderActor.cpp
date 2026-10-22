/**
 * @file ShaderActor.cpp
 * @brief Implementation and reflection metadata for ShaderActor.
 */

#include "ShaderActor.hpp"

namespace Engine {

    ShaderActor::ShaderActor()
        : ShaderActor("Shader Volume") {}

    ShaderActor::ShaderActor(std::string name)
        : Actor(std::move(name), /*bAttachDefaultComponents=*/false) {
        bIsVisible = false;
        AddComponent<ShaderComponent>();
    }

    ENGINE_CLASS_BEGIN(ShaderActor, Actor)
    ENGINE_CLASS_END()

} // namespace Engine
