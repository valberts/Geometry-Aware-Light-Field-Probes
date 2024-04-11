#pragma once
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

#define PI 3.1415926

glm::vec3 inverse_to_pos(glm::vec3 pos, glm::vec3 origin, float armLength);

void jacobianMatrix(float beta, float armLength, float theta1, float theta2, float theta3, float& J11, float& J12, float& J13,
	float& J21, float& J22, float& J23, float& J31, float& J32, float& J33);

void forwardKinematics(float beta, float armLength, float theta1, float theta2, float theta3, float& x, float& y, float& z);

void inverseKinematics(float beta, float armLength, glm::vec3 pos, glm::vec3 origin, float& theta1, float& theta2, float& theta3);

