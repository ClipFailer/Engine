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

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec4 fragBaseColor;
layout(location = 4) in vec3 fragCameraPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(fragCameraPos - fragWorldPos);

    // Subtle ambient light (sky & ground bounce)
    vec3 skyColor = vec3(0.08, 0.09, 0.12);
    vec3 groundColor = vec3(0.03, 0.03, 0.04);
    float hemi = (N.y + 1.0) * 0.5;
    vec3 ambient = mix(groundColor, skyColor, hemi);

    vec3 diffuseAccum = vec3(0.0);
    vec3 specAccum = vec3(0.0);

    // 1. Dynamic Directional Light (Sun)
    if (ubo.dirLightDirAndEnabled.w > 0.5) {
        vec3 L = normalize(-ubo.dirLightDirAndEnabled.xyz);
        float diff = max(dot(N, L), 0.0);
        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), 32.0);

        vec3 dirColor = ubo.dirLightColorAndIntensity.xyz * ubo.dirLightColorAndIntensity.w;
        diffuseAccum += diff * dirColor;
        specAccum += spec * 0.4 * dirColor;
    }

    // 2. Dynamic Point Lights (Omnidirectional point sources)
    for (int i = 0; i < ubo.pointLightCount && i < 4; ++i) {
        vec3 lightPos = ubo.pointLights[i].positionAndRadius.xyz;
        float radius = ubo.pointLights[i].positionAndRadius.w;
        vec3 lightColor = ubo.pointLights[i].colorAndIntensity.xyz * ubo.pointLights[i].colorAndIntensity.w;

        vec3 toLight = lightPos - fragWorldPos;
        float dist = length(toLight);

        if (dist < radius && radius > 0.001) {
            vec3 L = normalize(toLight);
            float atten = clamp(1.0 - (dist / radius), 0.0, 1.0);
            atten *= atten; // Quadratic attenuation curve

            float diff = max(dot(N, L), 0.0);
            vec3 H = normalize(L + V);
            float spec = pow(max(dot(N, H), 0.0), 32.0);

            diffuseAccum += diff * lightColor * atten;
            specAccum += spec * 0.4 * lightColor * atten;
        }
    }

    // Combined illumination with material base color
    vec3 lighting = ambient + diffuseAccum + specAccum;
    vec3 finalRgb = fragBaseColor.rgb * lighting;

    // Gamma correction
    finalRgb = pow(finalRgb, vec3(1.0 / 2.2));

    outColor = vec4(finalRgb, fragBaseColor.a);
}
