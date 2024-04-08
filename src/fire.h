#pragma once
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

#define	MAX_PARTICLES	2000 // TODO: for testing, change this to 1e4

// define colors
#define WHITE glm::vec3(1.0f, 1.0f, 1.0f)
#define BLUE glm::vec3(0.0f, 0.0f, 1.0f)
#define YELLOW glm::vec3(1.0f, 1.0f, 0.0f)
#define ORANGE glm::vec3(1.0f, 0.5f, 0.0f)
#define RED glm::vec3(1.0f, 0.1f, 0.0f)

typedef struct {
	bool	active;
	float	life;
	float	fade;
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 speed;
	glm::vec3 acceleration;
} Particle;

void initFire(Particle m_fire[], int size, const glm::vec3& minBounds, const glm::vec3& maxBounds);

void updateFire(Particle m_fire[], int size, const glm::vec3& minBounds, const glm::vec3& maxBounds);