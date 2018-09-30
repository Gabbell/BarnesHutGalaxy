#pragma once

#include <vector>

#include "CommonCelestials.h"

//#define DRAW_QUADTREE

class QuadTreeNode;

class Universe
{
public:
	Universe( GLuint numberOfGalaxies, GLuint numberOfStars );
	~Universe();

	void step(GLfloat deltaTime);

	std::vector<glm::vec3> getVertices() const;
	std::vector<glm::vec3> getQuadTreeVerts() const { return _quadTreeVerts; }

private:

	void buildTree();

	static constexpr GLfloat GALAXYRADIUS = 0.5;

	GLuint _numberOfGalaxies;
	std::vector<Galaxy> _galaxies;

	GLuint _numberOfStars;
	std::vector<Star> _stars;

	QuadTreeNode* _root;
	std::vector<glm::vec3> _quadTreeVerts;
};

