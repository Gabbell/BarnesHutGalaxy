#pragma once

#include <memory>
#include <array>
#include <vector>

#include "Common.h"
#include "Types.h"

class QuadTreeNode
{
public:
	QuadTreeNode(int nodeId, const float3 &min, const float3 &max, QuadTreeNode* parent = nullptr);

	enum class Quadrant : int
	{
		NE = 0,
		NW,
		SW,
		SE,
		NONE
	};

	// Inserts a star into the quad tree
	void insertStar(Star* const star);
	
	// Goes through the tree recursively and computes the total mass and center of mass
	// of all particles in all nodes of the quad tree 
	void computeMassDistribution();

	// Goes through the tree recursively and computes the approximate acceleration for
	//every star based on theta
	float3 calcTreeForce(const Star* const star);

	// Is called if one particle is present in a node and we have not satisfied the
	// theta approximation
	float3 calcAcc(const Star* const star1, const Star* const star2);

	// Traverse the tree and translates it into an array to be uploaded to the GPU
	void traverseAndPopulate(Node* toGpu);
	
	// Fetch vertices to draw the quad tree
	void getQuadTreeVerts(std::vector<float3>& quads);

	// References to quadrant nodes
	std::array<std::unique_ptr<QuadTreeNode>, 4> quadNodes;

	// Node id counter
	static int s_idCounter;

private:
	bool isRoot() const;
	bool isLeaf() const;

	// Creates a new quad tree node based on a quadrant if it doesn't already exist
	QuadTreeNode* createQuadNode(Quadrant quad);

	// Fetches a star's quadrant based on the node's center, min and max
	Quadrant getQuadrant(const Star* const star);

	int _id;

	double _mass;
	float3 _centerOfMass;
	float3 _min; // Lower left edge of node
	float3 _max; // Upper right edge of node
	float3 _center;
	QuadTreeNode* _parent;
	Star* _star;
	uint32 _numParticles;
};

