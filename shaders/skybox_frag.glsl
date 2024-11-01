#version 450

out vec4 fragColor;

in vec3 localPos;
  
layout(location = 2) uniform samplerCube environmentMap;

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
void main()
{
    vec3 envColor = texture(environmentMap, localPos).rgb;
    
    // HDR tonemapping and gamma correction
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 

    fragColor = vec4(envColor, 1.0);
}