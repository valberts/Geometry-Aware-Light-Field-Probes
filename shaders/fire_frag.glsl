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
layout(location = 9) uniform bool night;
layout(location = 10) uniform vec3 setColor;
layout(location = 12) uniform bool hasNormalMap;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(setColor, 1.0);
}
