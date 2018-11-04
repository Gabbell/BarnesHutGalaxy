#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

//#define DRAW_QUADTREE

// Adds one supermassive star per galaxy at its center
//#define ADD_BLACK_HOLE

static constexpr int BLOCKSIZE = 512;

static constexpr GLfloat PARTICLE_MASS = 1.0e5;
static constexpr GLfloat BLACK_HOLE_MASS = 1.0e8;
static constexpr GLfloat INITIAL_SPIN_FACTOR = 0.0f;
static constexpr double GRAVITATIONAL_CONSTANT = 6.67408e-11;
static constexpr double SOFTENER = 0.1;

struct Rect
{
	GLfloat x1; // Left
	GLfloat x2; // Right
	GLfloat y1; // Bottom
	GLfloat y2; // Top
};

struct Star
{
	Star()
		: mass(PARTICLE_MASS)
		, position(0.0f, 0.0f, 0.0f)
		, velocity(0.0f, 0.0f, 0.0f)
		, acceleration(0.0f, 0.0f, 0.0f)
	{}
	Star(GLfloat mass, glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration)
		: mass(mass)
		, position(position)
		, velocity(velocity)
		, acceleration(acceleration)
	{}
	GLfloat mass;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
};
