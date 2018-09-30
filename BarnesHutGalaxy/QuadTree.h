#pragma once

#include <memory>
#include <array>
#include <vector>

#include "CommonCelestials.h"

class QuadTreeNode
{
public:
	QuadTreeNode( const glm::vec3 &min, const glm::vec3 &max, QuadTreeNode* parent = nullptr );

	enum class Quadrant : int
	{
		NE = 0,
		NW,
		SW,
		SE,
		NONE
	};

	std::array<std::unique_ptr<QuadTreeNode>, 4> quadNodes;

	void insertStar(Star* const star);
	void getQuadTreeVerts(std::vector<glm::vec3>& quads);

	QuadTreeNode* createQuadNode(Quadrant quad);

	void computeMassDistribution();
	glm::vec3 calcTreeForce(const Star* const star);
	glm::vec3 calcAcc(const Star* const star1, const Star* const star2);

private:
	bool isRoot() const;
	bool isLeaf() const;

	Quadrant getQuadrant(const Star* const star);

	double _mass;
	glm::vec3 _centerOfMass;
	glm::vec3 _min; // Lower left edge of node
	glm::vec3 _max; // Upper right edge of node
	glm::vec3 _center;
	QuadTreeNode* _parent;
	Star* _star;
	GLuint _numParticles;

};

