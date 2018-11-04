#pragma once

#include <vector>

#include "Galaxy.h"

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
	GLuint _numberOfStars;			// Number of stars per galaxy
	GLuint _totalStars;
	std::vector<Galaxy> _galaxies;

	//Host
	std::vector<Star> _stars;

	//Device
	Star* _dinStars;
	Star* _doutStars;

	QuadTreeNode* _root;

	std::vector<glm::vec3> _quadTreeVerts;
};

