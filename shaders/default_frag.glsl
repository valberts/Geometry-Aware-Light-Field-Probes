#version 450

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

layout(location = 3) uniform sampler2D colorMap;
layout(location = 4) uniform bool hasTexCoords;
layout(location = 5) uniform sampler2D normalMap;
layout(location = 6) uniform bool hasNormalMap;
layout(location = 7) uniform sampler2D metallicMap;
layout(location = 8) uniform bool hasMetallicMap;
layout(location = 9) uniform float metallic;
layout(location = 10) uniform sampler2D roughnessMap;
layout(location = 11) uniform bool hasRoughnessMap;
layout(location = 12) uniform float roughness;
layout(location = 13) uniform vec3 albedo;
layout(location = 14) uniform bool useMaterial;
layout(location = 15) uniform vec3 lightPosition;
layout(location = 16) uniform vec3 lightColor;
layout(location = 17) uniform vec3 cameraPosition;
layout(location = 18) uniform float ao = 1.0f;

layout(location = 19) uniform samplerCube irradianceMap;
layout(location = 20) uniform bool useIrradianceMap;

layout(location = 30) uniform bool night;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in mat3 TBN;

layout(location = 0) out vec4 fragColor;

float PI = 3.14159265359;

// https://learnopengl.com/PBR/Lighting
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

void main()
{
    vec3 fragAlbedo = albedo;
    if (hasTexCoords) { // Check for diffuse texture
        fragAlbedo = texture(colorMap, fragTexCoord).rgb;
    }

    vec3 N;
    if (hasNormalMap) {
        vec3 normal = texture(normalMap, fragTexCoord).rgb;
        N = normalize(normal * 2.0 - 1.0); // Convert from [0, 1] to [-1, 1]
        N = normalize(TBN * N);
    } else {
        N = normalize(fragNormal);
    }

    // Vectors and attenuation
    vec3 V = normalize(cameraPosition - fragPosition);
    vec3 L = normalize(lightPosition - fragPosition);
    vec3 H = normalize(V + L);
    float distance = length(lightPosition - fragPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;

    // Distribution, Geometry, and Specular calculations
    float actualMetallic = hasMetallicMap ? texture(metallicMap, fragTexCoord).r : metallic;
    float actualRoughness = hasRoughnessMap ? texture(roughnessMap, fragTexCoord).r : roughness;

    // Fresnel
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, fragAlbedo, actualMetallic);
    vec3 F = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, actualRoughness);
    vec3 F0_metallic = mix(vec3(0.04), fragAlbedo, actualMetallic);

    float NDF = DistributionGGX(N, H, actualRoughness);       
    float G = GeometrySmith(N, V, L, actualRoughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;  

    // Reflectance equation
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - actualMetallic;  

    float NdotL = max(dot(N, L), 0.0);        
    vec3 outgoing = (kD * fragAlbedo / PI + specular) * radiance * NdotL;

    // Ambient lighting
    vec3 irradiance = vec3(0.05);
    if (useIrradianceMap) {
        irradiance = texture(irradianceMap, N).rgb;
        //fragColor = vec4(irradiance, 1.0); // debug irradiance
        //return;
    }

    // Night mode
    if (night) {
        irradiance *= 0.01;
    }

    vec3 diffuse    = irradiance * fragAlbedo;
    vec3 ambient    = (kD * diffuse) * ao; 
    vec3 color = ambient + outgoing;

    // HDR tonemapping and gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

    fragColor = vec4(color, 1.0);
}