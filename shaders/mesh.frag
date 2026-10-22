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
    ivec4 renderModes; // x: pointLightCount, y: lightingModel, z: toneMappingMode, w: antiAliasingMode
    vec4 renderParams; // x: exposure, y: gamma, z: aoStrength, w: bloomIntensity
    ivec4 shaderVolume; // x: active(0/1), y: effectType(1..6), z: unused, w: unused
    vec4 shaderParams; // x: intensity, y: param1, z: param2, w: param3
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

    float metallic = 0.15;
    float roughness = clamp(0.45, 0.05, 1.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    int lightingModel = ubo.renderModes.y; // 0: PBR, 1: Blinn-Phong, 2: Soft Irradiance

    // 1. Directional Sun Light
    if (ubo.dirLightDirAndEnabled.w > 0.5) {
        vec3 L = normalize(-ubo.dirLightDirAndEnabled.xyz);
        vec3 H = normalize(V + L);
        vec3 radiance = ubo.dirLightColorAndIntensity.xyz * ubo.dirLightColorAndIntensity.w;
        float NdotL = max(dot(N, L), 0.0);

        if (lightingModel == 0) {
            // PBR Cook-Torrance
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        } else if (lightingModel == 1) {
            // Blinn-Phong
            float diff = NdotL;
            float spec = pow(max(dot(N, H), 0.0), mix(16.0, 128.0, 1.0 - roughness));
            Lo += (albedo * diff + vec3(spec * 0.7)) * radiance;
        } else {
            // Soft Irradiance
            float wrap = max(0.0, (dot(N, L) + 0.5) / 1.5);
            Lo += albedo * radiance * wrap;
        }
    }

    // 2. Dynamic Point Lights
    for (int i = 0; i < ubo.renderModes.x && i < 4; ++i) {
        vec3 lightPos = ubo.pointLights[i].positionAndRadius.xyz;
        float radius = ubo.pointLights[i].positionAndRadius.w;
        vec3 lightColor = ubo.pointLights[i].colorAndIntensity.xyz * ubo.pointLights[i].colorAndIntensity.w;

        vec3 toLight = lightPos - fragWorldPos;
        float dist = length(toLight);

        if (dist < radius && radius > 0.001) {
            vec3 L = normalize(toLight);
            vec3 H = normalize(V + L);
            float atten = clamp(1.0 - (dist / radius), 0.0, 1.0);
            atten *= atten;
            vec3 radiance = lightColor * atten;
            float NdotL = max(dot(N, L), 0.0);

            if (lightingModel == 0) {
                float NDF = DistributionGGX(N, H, roughness);
                float G = GeometrySmith(N, V, L, roughness);
                vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
                vec3 specular = numerator / denominator;

                vec3 kS = F;
                vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
                Lo += (kD * albedo / PI + specular) * radiance * NdotL;
            } else if (lightingModel == 1) {
                float diff = NdotL;
                float spec = pow(max(dot(N, H), 0.0), mix(16.0, 128.0, 1.0 - roughness));
                Lo += (albedo * diff + vec3(spec * 0.7)) * radiance;
            } else {
                float wrap = max(0.0, (dot(N, L) + 0.5) / 1.5);
                Lo += albedo * radiance * wrap;
            }
        }
    }

    // 3. Ambient Irradiance & Ambient Occlusion (SSAO)
    vec3 skyColor = vec3(0.12, 0.15, 0.20);
    vec3 groundColor = vec3(0.04, 0.04, 0.05);
    float hemi = (N.y + 1.0) * 0.5;
    float aoStrength = ubo.renderParams.z;
    float ao = clamp(1.0 - aoStrength * (1.0 - max(N.y, 0.0) * 0.7), 0.05, 1.0);
    vec3 ambient = mix(groundColor, skyColor, hemi) * albedo * ao;

    // 4. HDR Bloom threshold addition
    float bloomIntensity = ubo.renderParams.w;
    if (bloomIntensity > 0.001) {
        float lum = dot(Lo, vec3(0.2126, 0.7152, 0.0722));
        if (lum > 0.75) {
            Lo += Lo * (lum - 0.75) * bloomIntensity * 2.0;
        }
    }

    vec3 color = ambient + Lo;

    // 5. Exposure Compensation
    float exposure = max(ubo.renderParams.x, 0.05);
    color *= exposure;

    // 6. Tone Mapping
    int toneMode = ubo.renderModes.z; // 0: ACES Filmic, 1: Reinhard, 2: Neutral, 3: AgX
    if (toneMode == 0) {
        color = ACESFilm(color);
    } else if (toneMode == 1) {
        color = color / (color + vec3(1.0));
    } else if (toneMode == 2) {
        color = clamp(color, 0.0, 1.0);
    } else if (toneMode == 3) {
        color = clamp(color * (color * 0.35 + 0.65), 0.0, 1.0);
    }

    // 7. Display Gamma Correction
    float gamma = max(ubo.renderParams.y, 0.1);
    color = pow(color, vec3(1.0 / gamma));

    // 8. Custom Shader Volume Trigger Processing
    if (ubo.shaderVolume.x == 1) {
        int effect = ubo.shaderVolume.y;
        float intensity = clamp(ubo.shaderParams.x, 0.0, 2.0);

        if (effect == 1) {
            // Vignette
            vec2 screenUV = gl_FragCoord.xy / vec2(1280.0, 720.0);
            vec2 coord = (screenUV - 0.5) * 2.0;
            float dist = length(coord);
            float vig = smoothstep(0.95, 0.25, dist);
            color = mix(color, color * vig, intensity);
        } else if (effect == 2) {
            // NightVision
            float lum = dot(color, vec3(0.299, 0.587, 0.114));
            vec3 nv = vec3(0.1, 0.95, 0.2) * (lum * 1.8 + 0.15);
            float scanline = sin(gl_FragCoord.y * 1.2) * 0.06;
            vec3 targetNV = clamp(nv + scanline, 0.0, 1.0);
            color = mix(color, targetNV, intensity);
        } else if (effect == 3) {
            // Grayscale
            float gray = dot(color, vec3(0.299, 0.587, 0.114));
            color = mix(color, vec3(gray), intensity);
        } else if (effect == 4) {
            // Sepia
            vec3 sepia;
            sepia.r = dot(color, vec3(0.393, 0.769, 0.189));
            sepia.g = dot(color, vec3(0.349, 0.686, 0.168));
            sepia.b = dot(color, vec3(0.272, 0.534, 0.131));
            color = mix(color, clamp(sepia, 0.0, 1.0), intensity);
        } else if (effect == 5) {
            // Invert
            vec3 inv = vec3(1.0) - color;
            color = mix(color, inv, intensity);
        } else if (effect == 6) {
            // Custom / Cyberpunk Tint
            vec3 tinted = color * vec3(0.35, 0.85, 1.3);
            color = mix(color, tinted, intensity);
        }
    }

    outColor = vec4(color, fragBaseColor.a);
}
