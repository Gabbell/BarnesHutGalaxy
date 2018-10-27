#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

//#define DRAW_QUADTREE

// Adds one supermassive star per galaxy at its center
//#define ADD_BLACK_HOLE

//Have only one of these at one time
//#define MT_IMP
#define TBB_IMP

static constexpr int NUMBER_OF_THREADS = 32;

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
