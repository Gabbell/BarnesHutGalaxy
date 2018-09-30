#include <iostream>

#include "QuadTree.h"

static constexpr double THETA = 1.0;
static constexpr double GRAVITATIONAL_CONSTANT = 6.67408e-11;
static constexpr double SOFTENER = 0.1*0.1;

QuadTreeNode::QuadTreeNode(const glm::vec3& min, const glm::vec3& max, QuadTreeNode* parent)
	: _min(min)
	, _max(max)
	, _center(min.x + (max.x - min.x) / 2.0, min.y + (max.y - min.y) / 2.0, 0.0)
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
	glm::vec3 pos = star->position;

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
		case Quadrant::SW: return new QuadTreeNode(_min, _center, this);
		case Quadrant::NW: return new QuadTreeNode(glm::vec3(_min.x, _center.y, 0.0),
			glm::vec3(_center.x, _max.y, 0.0),
			this);
		case Quadrant::NE: return new QuadTreeNode(_center, _max, this);
		case Quadrant::SE: return new QuadTreeNode(
			glm::vec3(_center.x, _min.y, 0.0),
			glm::vec3(_max.x, _center.y, 0.0),
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
		_centerOfMass = _star->position;
	}
	else
	{
		_mass = 0;
		_centerOfMass = glm::vec3(0.0f, 0.0f, 0.0f);

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

	glm::vec3 difference = star2->position - star1->position;

	double distance = glm::length(difference) + SOFTENER;

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
	glm::vec3 acc(0.0, 0.0, 0.0);

	if (_numParticles == 1)
	{
		acc = calcAcc(star, _star);
	}
	else
	{
		glm::vec3 difference = _centerOfMass - star->position;
		double distance = glm::length(difference) + 0.0000001;
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
		if (star->position == _star->position)
		{
			// Two particles are at the same position. This is bad
			assert(false);
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