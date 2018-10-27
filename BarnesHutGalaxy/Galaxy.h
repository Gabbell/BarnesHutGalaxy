#pragma once

#include "CommonCelestials.h"

#include "glm/glm.hpp"
#include "GL/glew.h"

#include <vector>

class QuadTreeNode;

struct Galaxy
{
	Galaxy() = default;
	Galaxy(GLfloat radius, glm::vec3 center);
	~Galaxy() = default;

	void step(GLfloat deltaTime);
	void initStars(GLuint numberOfStars);

	void setRadius(GLfloat radius) { _radius = radius; }
	void setCenter(glm::vec3 center) { _center = center; }
	void setRandomCenter();

	GLfloat getRadius() const { return _radius; }

	std::vector<glm::vec3> getVertices() const;

#ifdef DRAW_QUADTREE
	std::vector<glm::vec3> getQuadTreeVerts() const;
#endif

private:
	GLfloat _radius;
	glm::vec3 _center;
	glm::vec3 _colour;

	std::vector<Star> _stars;
	QuadTreeNode* _root;
};

