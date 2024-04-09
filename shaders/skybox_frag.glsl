#version 450

out vec4 fragColor;

in vec3 texCoords;

layout(location = 2) uniform samplerCube skybox;

void main() {
	fragColor = texture(skybox, texCoords);
}