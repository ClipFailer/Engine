#version 450

struct PointLightData {
    vec4 positionAndRadius;
    vec4 colorAndIntensity;
};

struct SpotLightData {
    vec4 positionAndRadius;
    vec4 directionAndInner;
    vec4 colorAndOuter;
};

layout(binding = 0) uniform SceneUBO {
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    vec4 dirLightDirAndEnabled;
    vec4 dirLightColorAndIntensity;
    PointLightData pointLights[4];
    SpotLightData spotLights[2];
    ivec4 renderModes; // x: pointLightCount, y: lightingModel, z: toneMappingMode, w: antiAliasingMode
    vec4 renderParams; // x: exposure, y: gamma, z: aoStrength, w: bloomIntensity
    ivec4 shaderVolume; // x: active(0/1), y: effectType(1..6), z: spotLightCount, w: unused
    vec4 shaderParams; // x: intensity, y: param1, z: param2, w: param3
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 baseColor;
    float metallic;
    float roughness;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec4 fragBaseColor;
layout(location = 4) out vec3 fragCameraPos;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    
    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));
    fragNormal = normalize(normalMatrix * inNormal);
    
    fragTexCoord = inTexCoord;
    fragBaseColor = pc.baseColor;
    fragCameraPos = ubo.cameraPos.xyz;

    gl_Position = ubo.proj * ubo.view * worldPos;
}
