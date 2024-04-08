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
layout(location = 5) uniform bool useMaterial;
layout(location = 6) uniform int shadingMode;
layout(location = 7) uniform vec3 lightPos;
layout(location = 8) uniform vec3 lightColor;
layout(location = 11) uniform sampler2D normalMap;
layout(location = 12) uniform bool hasNormalMap;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 normal = vec3(0.0f);
    if (hasNormalMap) {
    normal = texture(normalMap, fragTexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    } else {
        normal = fragNormal;
    }
    //const vec3 lightColor = vec3(1.0, 1.0, 1.0); 
    vec3 lightDir = normalize(lightPos - fragPosition);
    
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * lightColor;

    float distanceToLight = length(lightPos - fragPosition);
    // defined static attenuation factors
    float attenuation = 1.0 / (1.0 + 0.09 * distanceToLight + 0.032 * distanceToLight * distanceToLight);

    vec3 diffuseWithAttenuation = diffuse * attenuation;

    if (shadingMode == 0) {
        vec3 baseColor;
        if (hasTexCoords)       { baseColor = texture(colorMap, fragTexCoord).rgb;}
        else if (useMaterial)   { baseColor = kd;}
        else                    { baseColor = normal; } // Output color value, change from (1, 0, 0) to something else

        fragColor = vec4(diffuseWithAttenuation * baseColor, 1.0);
    } else {
        fragColor = vec4(1.0);
    }
}
