/**
 * @file ShaderComponent.cpp
 * @brief Implementation and reflection metadata for ShaderComponent.
 */

#include "ShaderComponent.hpp"
#include "core/Log.hpp"
#include <glm/geometric.hpp>

namespace Engine {

    std::string ShaderComponent::GetDefaultShaderCode() {
        return R"(#version 450
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D sceneColor;

void main() {
    vec4 color = texture(sceneColor, inUV);
    // Custom post-processing shader logic
    vec2 coord = (inUV - 0.5) * 2.0;
    float dist = length(coord);
    float vignette = smoothstep(0.9, 0.3, dist);
    outColor = vec4(color.rgb * vignette, color.a);
}
)";
    }

    ShaderComponent::ShaderComponent()
        : Component("ShaderComponent") {
        ShaderCode = GetDefaultShaderCode();
    }

    bool ShaderComponent::IsActiveAtPosition(const glm::vec3& cameraPos, const glm::vec3& actorPos) const noexcept {
        if (bActiveEverywhere) {
            return true;
        }

        glm::vec3 center = actorPos + TriggerOffset;
        if (bIsSphereZone) {
            float distSq = glm::dot(cameraPos - center, cameraPos - center);
            return distSq <= (TriggerRadius * TriggerRadius);
        } else {
            glm::vec3 diff = glm::abs(cameraPos - center);
            return (diff.x <= TriggerHalfExtents.x &&
                    diff.y <= TriggerHalfExtents.y &&
                    diff.z <= TriggerHalfExtents.z);
        }
    }

    void ShaderComponent::LoadPreset(const std::string& presetName) {
        if (presetName == "Vignette") {
            ShaderCode = R"(#version 450
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D sceneColor;

void main() {
    vec4 color = texture(sceneColor, inUV);
    vec2 coord = (inUV - 0.5) * 2.0;
    float dist = length(coord);
    float vignette = smoothstep(0.85, 0.25, dist);
    outColor = vec4(color.rgb * vignette, color.a);
}
)";
        } else if (presetName == "NightVision") {
            ShaderCode = R"(#version 450
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D sceneColor;

void main() {
    vec4 color = texture(sceneColor, inUV);
    float lum = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    vec3 nv = vec3(0.1, 0.95, 0.2) * (lum * 1.5 + 0.12);
    outColor = vec4(nv, color.a);
}
)";
        } else if (presetName == "Grayscale") {
            ShaderCode = R"(#version 450
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D sceneColor;

void main() {
    vec4 color = texture(sceneColor, inUV);
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    outColor = vec4(vec3(gray), color.a);
}
)";
        } else if (presetName == "Sepia") {
            ShaderCode = R"(#version 450
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D sceneColor;

void main() {
    vec4 color = texture(sceneColor, inUV);
    vec3 sepia;
    sepia.r = dot(color.rgb, vec3(0.393, 0.769, 0.189));
    sepia.g = dot(color.rgb, vec3(0.349, 0.686, 0.168));
    sepia.b = dot(color.rgb, vec3(0.272, 0.534, 0.131));
    outColor = vec4(sepia, color.a);
}
)";
        } else if (presetName == "Invert") {
            ShaderCode = R"(#version 450
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D sceneColor;

void main() {
    vec4 color = texture(sceneColor, inUV);
    outColor = vec4(vec3(1.0) - color.rgb, color.a);
}
)";
        } else {
            LOG_WARN("ShaderComponent: Unknown preset '{}'", presetName);
        }
    }

    ENGINE_CLASS_BEGIN(ShaderComponent, Component)
        ENGINE_PROPERTY(ShaderComponent, ShaderName, String, "Shader", "Shader Identifier")
        ENGINE_PROPERTY(ShaderComponent, bActiveEverywhere, Bool, "Trigger", "Activate across the entire scene")
        ENGINE_PROPERTY(ShaderComponent, bIsSphereZone, Bool, "Trigger", "Sphere zone shape if true, Box if false")
        ENGINE_PROPERTY(ShaderComponent, TriggerHalfExtents, Vec3, "Trigger", "Box trigger half-dimensions")
        ENGINE_PROPERTY(ShaderComponent, TriggerRadius, Float, "Trigger", "Sphere trigger radius")
        ENGINE_PROPERTY(ShaderComponent, TriggerOffset, Vec3, "Trigger", "Trigger center offset relative to Actor")
        ENGINE_PROPERTY(ShaderComponent, ShaderCode, String, "Shader", "GLSL fragment shader source code")
    ENGINE_CLASS_END()

} // namespace Engine
