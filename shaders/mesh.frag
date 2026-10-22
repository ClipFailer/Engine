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

const float PI = 3.14159265359;

// GGX / Trowbridge-Reitz normal distribution
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 0.000001);
}

// Schlick-GGX geometry shadowing
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

// Fresnel-Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ACES Filmic Tone Mapping Curve
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(fragCameraPos - fragWorldPos);
    vec3 albedo = fragBaseColor.rgb;

    // Default physical material factors (or derived from vertex inputs)
    float metallic = 0.15;
    float roughness = clamp(0.45, 0.05, 1.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    // 1. Directional Sun Light
    if (ubo.dirLightDirAndEnabled.w > 0.5) {
        vec3 L = normalize(-ubo.dirLightDirAndEnabled.xyz);
        vec3 H = normalize(V + L);
        vec3 radiance = ubo.dirLightColorAndIntensity.xyz * ubo.dirLightColorAndIntensity.w;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // 2. Dynamic Point Lights
    for (int i = 0; i < ubo.pointLightCount && i < 4; ++i) {
        vec3 lightPos = ubo.pointLights[i].positionAndRadius.xyz;
        float radius = ubo.pointLights[i].positionAndRadius.w;
        vec3 lightColor = ubo.pointLights[i].colorAndIntensity.xyz * ubo.pointLights[i].colorAndIntensity.w;

        vec3 toLight = lightPos - fragWorldPos;
        float dist = length(toLight);

        if (dist < radius && radius > 0.001) {
            vec3 L = normalize(toLight);
            vec3 H = normalize(V + L);

            float atten = clamp(1.0 - (dist / radius), 0.0, 1.0);
            atten *= atten; // Quadratic attenuation
            vec3 radiance = lightColor * atten;

            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
            float NdotL = max(dot(N, L), 0.0);

            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }
    }

    // 3. Ambient Irradiance (Atmospheric sky and ground hemisphere bounce)
    vec3 skyColor = vec3(0.10, 0.12, 0.16);
    vec3 groundColor = vec3(0.04, 0.04, 0.05);
    float hemi = (N.y + 1.0) * 0.5;
    vec3 ambient = mix(groundColor, skyColor, hemi) * albedo;

    vec3 color = ambient + Lo;

    // ACES Filmic Tone Mapping and Gamma correction for rich contrast
    color = ACESFilm(color);
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, fragBaseColor.a);
}
