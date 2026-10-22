#version 450

struct PointLightData {
    vec4 positionAndRadius;
    vec4 colorAndIntensity;
};

layout(binding = 0) uniform SceneUBO {
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    vec4 dirLightDirAndEnabled;
    vec4 dirLightColorAndIntensity;
    PointLightData pointLights[4];
    int pointLightCount;
    int pad1;
    int pad2;
    int pad3;
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
