#pragma once

#include "CommonCelestials.h"

#include "glm/glm.hpp"

struct Galaxy
{
	Galaxy() = default;
	Galaxy(GLfloat radius, glm::vec3 center);
	~Galaxy() = default;

	GLfloat radius;
	glm::vec3 center;
	glm::vec3 colour;
};

