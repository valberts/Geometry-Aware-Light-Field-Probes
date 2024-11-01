#version 450

layout(location = 0) uniform mat4 mvpMatrix;

in vec3 position;

void main()
{
    gl_Position =  mvpMatrix * vec4(position, 1.0);
}
