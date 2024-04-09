#version 450

layout(location = 0) in vec3 aPos;

out vec3 texCoords;

layout(location = 0) uniform mat4 projection;
layout(location = 1) uniform mat4 view;

void main() {
	vec4 pos = projection * view * vec4(aPos, 1.0);
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	texCoords = vec3(aPos.x, aPos.y, -aPos.z);
}