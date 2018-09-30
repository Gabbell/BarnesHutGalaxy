#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "Universe.h"

#include "QuadTree.h"

static constexpr double PARTICLE_MASS = 1.0e5;

float float_rand(float min, float max)
{
	float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
	return min + scale * (max - min);      /* [min, max] */
}

Universe::Universe( GLuint numberOfGalaxies, GLuint numberOfStars )
	: _numberOfGalaxies(numberOfGalaxies), _numberOfStars(numberOfStars)
{
	srand(time(NULL));

	_stars.reserve(_numberOfStars * _numberOfGalaxies);

	for ( GLuint i = 0; i < _numberOfGalaxies; i++ )
	{
		_galaxies.push_back({});
		_galaxies.at(i).galaxyCenter = { float_rand(-1,1), float_rand(-1,1), 0.0f };
		
		for ( GLuint j = 0 ; j <_numberOfStars; j++ )
		{
			float a = float_rand(0, 1) * 2 * M_PI;
			float r = GALAXYRADIUS * sqrt(float_rand(0, 1));

			float displacementX = r * cos(a);
			float displacementY = r * sin(a);

			Star star;
			star.position = { _galaxies.at(i).galaxyCenter.x + displacementX, _galaxies.at(i).galaxyCenter.y + displacementY, 0.0f };
			star.mass = PARTICLE_MASS;
			_stars.push_back(star);
		}
	}
}

void Universe::step( GLfloat deltaTime )
{
	glm::vec3 min(FLT_MAX, FLT_MAX, 0.0);
	glm::vec3 max(-FLT_MAX, -FLT_MAX, 0.0);

	for (const auto& star : _stars)
	{
		if (star.position.x < min.x)
		{
			min.x = star.position.x;
		}
		if (star.position.y < min.y)
		{
			min.y = star.position.y;
		}
		if (star.position.x > max.x)
		{
			max.x = star.position.x;
		}
		if (star.position.y > max.y)
		{
			max.y = star.position.y;
		}
	}

	_root = new QuadTreeNode(min, max, nullptr);

	buildTree();

	_root->computeMassDistribution();
	
	for (auto& galaxy : _galaxies)
	{
		for (auto& star : _stars)
		{
			star.acceleration = _root->calcTreeForce(&star);
		}
	}

	for (auto& galaxy : _galaxies)
	{
		for (auto& star : _stars)
		{
			const auto& acc = star.acceleration;
			star.velocity = star.velocity + glm::vec3(acc.x*deltaTime, acc.y*deltaTime, 0.0f);
			const auto& vel = star.velocity;
			star.position = star.position + glm::vec3(vel.x*deltaTime, vel.y*deltaTime, 0.0f);
		}
	}

#ifdef DRAW_QUADTREE
	_quadTreeVerts.clear();
	_root->getQuadTreeVerts( _quadTreeVerts );
#endif

	delete _root;
}

void Universe::buildTree()
{
	for (auto& star : _stars)
	{
		_root->insertStar(&star);
	}
}

std::vector<glm::vec3> Universe::getVertices() const
{
	std::vector<glm::vec3> verts(_stars.size());
	for (GLuint i = 0; i < verts.size(); i++)
	{
		verts.at(i) = _stars.at(i).position;
	}
	return verts;
}

Universe::~Universe()
{
}
