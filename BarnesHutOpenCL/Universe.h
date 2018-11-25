#pragma once

#include "Galaxy.h"
#include "Types.h"
#include "OpenCLHelper.h"

#include <vector>

#include "glm/glm.hpp"

class QuadTreeNode;

class Universe
{
public:
	Universe(uint32 numberOfGalaxies, float32 galaxyRadius, uint32 numberOfStars);
	~Universe();

	void step(float32 deltaTime);

	std::vector<glm::vec3> getVertices() const;
	std::vector<glm::vec3> getQuadTreeVerts() const { return _quadTreeVerts; }

private:
	OpenCLHelper _oclh;
	
	uint32 _numberOfGalaxies;		// Total number of galaxies
	uint32 _numberOfStars;			// Number of stars per galaxy
	cl_int _totalStars;				// Total number of stars in the universe

	std::vector<Galaxy> _galaxies;

	// QuadTree
	QuadTreeNode* _root;
	std::vector<glm::vec3> _quadTreeVerts;

	// Host
	std::vector<Star> _stars;
};

