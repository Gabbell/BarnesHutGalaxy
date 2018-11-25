#include "QuadTree.h"
#include "Constants.h"

#include "CL/cl.h"

#include <iostream>
#include <cassert>

int QuadTreeNode::s_idCounter = 0;

QuadTreeNode::QuadTreeNode(int nodeId, const glm::vec3& min, const glm::vec3& max, QuadTreeNode* parent)
	: _id(nodeId)
	, _min(min)
	, _max(max)
	, _center({ min.x + (max.x - min.x) / 2.0f, min.y + (max.y - min.y) / 2.0f, 0.0f })
	, _parent(parent)
	, _star(nullptr)
	, _numParticles(0)
{
	for (int i = 0; i < 4; i++)
	{
		quadNodes[i].reset(nullptr);
	}
}

QuadTreeNode::Quadrant QuadTreeNode::getQuadrant(const Star* const star)
{
	glm::vec3 pos = { star->position.x, star->position.y, star->position.z };

	if (pos.x <= _center.x && pos.y <= _center.y)
	{
		return Quadrant::SW;
	}
	else if (pos.x <= _center.x && pos.y >= _center.y)
	{
		return Quadrant::NW;
	}
	else if (pos.x >= _center.x && pos.y >= _center.y)
	{
		return Quadrant::NE;
	}
	else if (pos.x >= _center.x && pos.y <= _center.y)
	{
		return Quadrant::SE;
	}
	else
	{
		throw std::runtime_error("Can't determine quadrant");
	}
}

bool QuadTreeNode::isRoot() const
{
	return _parent == nullptr;
}

bool QuadTreeNode::isLeaf() const
{
	return quadNodes[0] == nullptr &&
		quadNodes[1] == nullptr &&
		quadNodes[2] == nullptr &&
		quadNodes[3] == nullptr;
}

QuadTreeNode* QuadTreeNode::createQuadNode(Quadrant quad)
{
	switch (quad)
	{
		case Quadrant::SW: return new QuadTreeNode(++s_idCounter, _min, _center, this);
		case Quadrant::NW: return new QuadTreeNode(++s_idCounter, { _min.x, _center.y, 0.0 },
			{ _center.x, _max.y, 0.0 },
			this);
		case Quadrant::NE: return new QuadTreeNode(++s_idCounter, _center, _max, this);
		case Quadrant::SE: return new QuadTreeNode(++s_idCounter,
			{ _center.x, _min.y, 0.0 },
			{ _max.x, _center.y, 0.0 },
			this);
		default:
		{
			throw std::runtime_error("Can't determine quadrant");
		}
	}
}

void QuadTreeNode::getQuadTreeVerts(std::vector<glm::vec3>& quads)
{
	quads.push_back({ _min.x, _min.y, 0.0f });
	quads.push_back({ _min.x, _max.y, 0.0f });
	quads.push_back({ _max.x, _max.y, 0.0f });
	quads.push_back({ _max.x, _min.y, 0.0f });

	if (quadNodes[0]) // NE
	{
		quadNodes[0]->getQuadTreeVerts(quads);
	}
	if (quadNodes[1]) // NW
	{
		quadNodes[1]->getQuadTreeVerts(quads);
	}
	if (quadNodes[2]) // SW
	{
		quadNodes[2]->getQuadTreeVerts(quads);
	}
	if (quadNodes[3]) // SE
	{
		quadNodes[3]->getQuadTreeVerts(quads);
	}
}

void QuadTreeNode::computeMassDistribution()
{
	if (_numParticles == 1)
	{
		_mass = _star->mass;
		cl_float3 starPos = _star->position;
		_centerOfMass = { starPos.x, starPos.y, starPos.z };
	}
	else
	{
		_mass = 0;
		_centerOfMass = { 0.0f, 0.0f, 0.0f };

		for (int i = 0; i < 4; ++i)
		{
			if (quadNodes[i])
			{
				quadNodes[i]->computeMassDistribution();
				_mass += quadNodes[i]->_mass;
				_centerOfMass.x += quadNodes[i]->_centerOfMass.x * quadNodes[i]->_mass;
				_centerOfMass.y += quadNodes[i]->_centerOfMass.y * quadNodes[i]->_mass;
			}
		}

		_centerOfMass.x /= _mass;
		_centerOfMass.y /= _mass;
	}
}

glm::vec3 QuadTreeNode::calcAcc(const Star* const star1, const Star* const star2)
{
	glm::vec3 acc;

	if (&star1 == &star2)
		return acc;

	glm::vec3 starPos1 = { star1->position.x, star1->position.y, star1->position.z };
	glm::vec3 starPos2 = { star2->position.x, star2->position.y, star2->position.z };
	glm::vec3 difference = starPos2 - starPos1;

	double distance = length(difference) + SOFTENER;

	if (distance > 0)
	{
		double g = GRAVITATIONAL_CONSTANT * star2->mass / (distance*distance*distance);

		acc.x += g * difference.x;
		acc.y += g * difference.y;
	}
	else
	{
		// Two particles in the same place
		acc.x = acc.y = 0;
	}

	return acc;
}
glm::vec3 QuadTreeNode::calcTreeForce(const Star* const star)
{
	glm::vec3 acc = { 0.0f, 0.0f, 0.0f };

	if (_numParticles == 1)
	{
		acc = calcAcc(star, _star);
	}
	else
	{
		glm::vec3 starPos = { star->position.x, star->position.y, star->position.z };

		glm::vec3 difference = _centerOfMass - starPos;
		double distance = length(difference) + SOFTENER;
		double nodeWidth = _max.x - _min.x;

		if (nodeWidth / distance <= THETA)
		{
			const double g = GRAVITATIONAL_CONSTANT * _mass / (distance*distance*distance);
			acc.x = g * difference.x;
			acc.y = g * difference.y;
		}
		else
		{
			for (int q = 0; q < 4; q++)
			{
				if (quadNodes[q])
				{
					const auto accQuad = quadNodes[q]->calcTreeForce(star);
					acc += accQuad;
				}
			}
		}
	}

	return acc;
}

void QuadTreeNode::insertStar(Star* const star)
{
	const auto& position = star->position;

	// Particle position is outside tree node
	assert(!((position.x < _min.x || position.x > _max.x) || (position.y < _min.y || position.y > _max.y)));

	if (_numParticles > 1)
	{
		const auto quad = getQuadrant(star);
		const int quadIndex = static_cast<std::underlying_type_t<Quadrant>>(quad);
		if (!quadNodes[quadIndex])
		{
			quadNodes[quadIndex].reset( createQuadNode(quad) );
		}
		quadNodes[quadIndex]->insertStar(star);
	}
	else if (_numParticles == 1)
	{
		if (star->position.x == _star->position.x &&
			star->position.y == _star->position.y &&
			star->position.z == _star->position.z)
		{
			// Two particles are at the same position. This is bad
			// assert(false);
			//s_renegades.push_back(newParticle);
		}
		else
		{
			// There is already a particle. We need to subdivide the node and relocate the particle.
			auto quad = getQuadrant(_star);
			auto quadIndex = static_cast<std::underlying_type_t<Quadrant>>(quad);
			if (!quadNodes[quadIndex])
			{
				quadNodes[quadIndex].reset(createQuadNode(quad));
			}
			quadNodes[quadIndex]->insertStar(_star);
			_star = nullptr;

			quad = getQuadrant(star);
			quadIndex = static_cast<std::underlying_type_t<Quadrant>>(quad);

			if (!quadNodes[quadIndex])
			{
				quadNodes[quadIndex].reset(createQuadNode(quad));
			}
			quadNodes[quadIndex]->insertStar(star);
		}
	}
	else if (_numParticles == 0)
	{
		_star = star;
	}

	_numParticles++;
}

void QuadTreeNode::traverseAndPopulate(Node* toGpu)
{
	// We get the equivalent index of the QuadTreeNode in the array version of the tree
	int nodeIndex = _id;
	toGpu[nodeIndex].mass = _mass;
	toGpu[nodeIndex].centerOfMass = { _centerOfMass.x, _centerOfMass.y, _centerOfMass.z };

	if (_numParticles == 1)
	{
		// If there is only one star in the node, this is a leaf node
		// Assign a star to it and then set its children to invalid
		toGpu[nodeIndex].starIndex = _star->id;
		for (uint32 i = 0; i < 4; i++)
		{
			toGpu[nodeIndex].children[i] = -1;
		}
	}
	else
	{
		// If we still have more than one star, we keep building the tree
		// We know we do not have one star so we set the star index to invalid
		// We must at least have one valid child so we should go through the if statement at least once
		// We then use recursion to keep traversing the tree
		toGpu[nodeIndex].starIndex = -1;
		for (uint32 i = 0; i < 4; i++)
		{
			if (quadNodes[i])
			{
				toGpu[nodeIndex].children[i] = quadNodes[i]->_id;
				quadNodes[i]->traverseAndPopulate(toGpu);
			}
			else
			{
				toGpu[nodeIndex].children[i] = -1;
			}
		}
	}
}