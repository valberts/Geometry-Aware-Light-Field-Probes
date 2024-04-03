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
layout(location = 7) uniform vec3 lightPos = vec3(0.0f, 10.0f, 10.0f);

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
    const vec3 normal = normalize(fragNormal);
    const vec3 lightColor = vec3(1.0, 1.0, 1.0); 
    vec3 lightDir = normalize(lightPos - fragPosition);
    
    vec3 diffuse = max(dot(fragNormal, lightDir), 0.0) * lightColor;

    if (shadingMode == 0) {
        fragColor = vec4(diffuse, 1.0f);
    } else {
        if (hasTexCoords)       { fragColor = vec4(texture(colorMap, fragTexCoord).rgb, 1);}
        else if (useMaterial)   { fragColor = vec4(kd, 1);}
        else                    { fragColor = vec4(normal, 1); } // Output color value, change from (1, 0, 0) to something else
    }
}
