#include "inversek.h"
#include <iostream>

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

void jacobianMatrix(float beta, float armLength, float theta1, float theta2, float theta3, float& J11, float& J12, float& J13,
	float& J21, float& J22, float& J23, float& J31, float& J32, float& J33) {
	J11 = -sin(beta) * armLength * (sin(theta1) + sin(theta1 + theta2) + sin(theta1 + theta2 + theta3));
	J12 = -sin(beta) * armLength * (sin(theta1 + theta2) + sin(theta1 + theta2 + theta3));
	J13 = -sin(beta) * armLength * sin(theta1 + theta2 + theta3);
	J21 = armLength * (cos(theta1) + cos(theta1 + theta2) + cos(theta1 + theta2 + theta3));
	J22 = armLength * (cos(theta1 + theta2) + cos(theta1 + theta2 + theta3));
	J23 = armLength * cos(theta1 + theta2 + theta3);
	J31 = -cos(beta) * armLength * (sin(theta1) + sin(theta1 + theta2) + sin(theta1 + theta2 + theta3));
	J32 = -cos(beta) * armLength * (sin(theta1 + theta2) + sin(theta1 + theta2 + theta3));
	J33 = -cos(beta) * armLength * sin(theta1 + theta2 + theta3);
}

void forwardKinematics(float beta, float armLength, float theta1, float theta2, float theta3, float& x, float& y, float& z) {
	x = sin(beta) * armLength * (cos(theta1) + cos(theta1 + theta2) + cos(theta1 + theta2 + theta3));
	y = armLength * (sin(theta1) + sin(theta1 + theta2) + sin(theta1 + theta2 + theta3));
    z = cos(beta) * armLength * (cos(theta1) + cos(theta1 + theta2) + cos(theta1 + theta2 + theta3));
}

void inverseKinematics(float beta, float armLength, glm::vec3 pos, glm::vec3 origin, float& theta1, float& theta2, float& theta3) {
    const float lambda = 0.01f; // Step size for updating joint angles
    const int maxIterations = 200;
    float x, y, z;
    glm::vec3 relativePos = pos - origin;

    for (int i = 0; i < maxIterations; ++i) {
        // Calculate current end-effector position
        forwardKinematics(beta, armLength, theta1, theta2, theta3, x, y, z);

        // Calculate error
        float errorX = relativePos.x - x;
        float errorY = relativePos.y - y;
        float errorZ = relativePos.z - z;

        // Check for convergence
        float error = sqrt(errorX * errorX + errorY * errorY + errorZ * errorZ);
        if (error < 0.001f) {
            break;
        }

        float J11, J12, J13, J21, J22, J23, J31, J32, J33;
        jacobianMatrix(beta, armLength, theta1, theta2, theta3, J11, J12, J13, J21, J22, J23, J31, J32, J33);

        float deltaX = lambda * (J11 * errorX + J12 * errorY + J13 * errorZ);
        float deltaY = lambda * (J21 * errorX + J22 * errorY + J23 * errorZ);
        float deltaZ = lambda * (J31 * errorX + J32 * errorY + J33 * errorZ);

        theta1 += deltaX;
        theta2 += deltaY;
        theta3 += deltaZ;
    }
}