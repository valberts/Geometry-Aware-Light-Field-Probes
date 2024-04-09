#version 450

layout (location = 0) in vec3 aPos;

layout(location = 0) uniform mat4 projection;
layout(location = 1) uniform mat4 view;

out vec3 localPos;

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
void main()
{
    localPos = aPos;

    mat4 rotView = mat4(mat3(view)); // remove translation from the view matrix
    vec4 clipPos = projection * rotView * vec4(localPos, 1.0);

    gl_Position = clipPos.xyww;
}