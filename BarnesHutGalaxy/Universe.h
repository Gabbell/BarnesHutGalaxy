#pragma once

#include <vector>

#include "CommonCelestials.h"
#include "ThreadPool.h"

//#define DRAW_QUADTREE

// Adds one supermassive star per galaxy at its center
//#define ADD_BLACK_HOLE

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

	void buildTree();

	GLuint _numberOfGalaxies;
	GLfloat _galaxyRadius;
	std::vector<Galaxy> _galaxies;

	GLuint _numberOfStars;
	std::vector<Star> _stars;

	QuadTreeNode* _root;
	std::vector<glm::vec3> _quadTreeVerts;

	ThreadPool _threadPool;
};

