#version 450

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;
// Normals should be transformed differently than positions:
// https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
layout(location = 2) uniform mat3 normalModelMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 tangent;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out mat3 TBN; // used to transform sampled normal map to correct space

void main()
{
    // Calculate the TBN matrix
    vec3 N = normalize(normal);
    vec3 T = normalize(tangent);
    // Calculate bitangent
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

    gl_Position = mvpMatrix * vec4(position, 1);
    
    fragPosition = (modelMatrix * vec4(position, 1)).xyz;
    fragNormal = normalModelMatrix * normal;
    fragTexCoord = texCoord;
}
