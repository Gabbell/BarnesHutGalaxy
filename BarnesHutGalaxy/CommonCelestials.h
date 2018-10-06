#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

struct Rect
{
	GLfloat x1; // Left
	GLfloat x2; // Right
	GLfloat y1; // Bottom
	GLfloat y2; // Top
};

struct Star
{
	GLfloat mass;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
};

struct Galaxy
{
	GLfloat radius;
	glm::vec3 center;
	glm::vec3 colour;
};