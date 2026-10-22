#version 450

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec4 fragBaseColor;
layout(location = 4) in vec3 fragCameraPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(fragCameraPos - fragWorldPos);

    // Primary directional light (Unreal-style sun)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 lightColor = vec3(1.0, 0.98, 0.92);

    // Diffuse Lambertian
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Blinn-Phong
    vec3 H = normalize(lightDir + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0);
    vec3 specular = spec * vec3(1.0) * 0.4;

    // Ambient light (soft sky and ground bounce)
    vec3 skyColor = vec3(0.2, 0.25, 0.35);
    vec3 groundColor = vec3(0.08, 0.07, 0.06);
    float hemi = (N.y + 1.0) * 0.5;
    vec3 ambient = mix(groundColor, skyColor, hemi);

    // Combined lighting
    vec3 lighting = ambient + diffuse + specular;
    vec3 finalRgb = fragBaseColor.rgb * lighting;

    // Gamma correction approximation
    finalRgb = pow(finalRgb, vec3(1.0 / 2.2));

    outColor = vec4(finalRgb, fragBaseColor.a);
}
