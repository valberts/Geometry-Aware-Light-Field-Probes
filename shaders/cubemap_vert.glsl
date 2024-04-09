#version 450 core
layout (location = 0) in vec3 aPos;

out vec3 localPos;

layout(location = 0) uniform mat4 projection;
layout(location = 1) uniform mat4 view;

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
void main()
{
    localPos = aPos;  
    gl_Position =  projection * view * vec4(localPos, 1.0);
}