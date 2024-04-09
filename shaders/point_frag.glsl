#version 450 core

layout(location = 0) out vec4 fragColor;
layout (location = 2) uniform vec3 pointColor;

void main()
{
    fragColor = vec4(pointColor, 1.0);
}
