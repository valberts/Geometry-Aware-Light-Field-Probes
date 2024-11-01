#version 450 core

layout(location = 2) uniform float near; // Near clipping plane
layout(location = 3) uniform float far;  // Far clipping plane

out vec4 fragColor;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    fragColor = vec4(vec3(depth), 1.0);
}