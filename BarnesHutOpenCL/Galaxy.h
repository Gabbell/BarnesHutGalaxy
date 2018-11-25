#pragma once

#include "Common.h"
#include "Types.h"

#include "glm/glm.hpp"

struct Galaxy
{
	Galaxy() = default;
	Galaxy(float32 radius, glm::vec3 center);
	~Galaxy() = default;

	float32 radius;
	glm::vec3 center;
	glm::vec3 colour;
};

