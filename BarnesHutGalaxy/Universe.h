#pragma once

#include <vector>

#include "Galaxy.h"
#include "ThreadPool.h"

class QuadTreeNode;

class Universe
{
public:
	Universe( GLuint numberOfGalaxies, GLfloat galaxyRadius, GLuint numberOfStars );
	~Universe();

	void step(float deltaTime);

	std::vector<glm::vec3> getVertices() const;
	std::vector<glm::vec3> getQuadTreeVerts() const { return _quadTreeVerts; }

private:

	GLuint _numberOfGalaxies;		// Total number of galaxies
	std::vector<Galaxy> _galaxies;

	std::vector<glm::vec3> _quadTreeVerts;
};

