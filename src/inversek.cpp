#include "inversek.h"

glm::vec3 inverse_to_pos(glm::vec3 pos, glm::vec3 origin, float armLength) {
	// calculate the angles needed to move from origin to pos
	// robot arm length predefined
	glm::vec3 relativePos = pos - origin;
	float b = atan2(relativePos.x, relativePos.z) * (180 / PI);
	float l = sqrt(relativePos.z * relativePos.z + relativePos.x * relativePos.x);
	float h = sqrt(l * l + relativePos.y * relativePos.y);
	float phi = atan(relativePos.y / l) * (180 / PI);
	float theta = acos(h / (2 * armLength)) * (180 / PI);
	float a1 = phi + theta;
	float a2 = phi - theta;
	float a3 = a2 - a1;
	return glm::vec3(b, a1, a3);
}