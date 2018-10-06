#include <stdlib.h>
#include <math.h>
#include <time.h>

//REMOVE
#include <iostream>

#include "Universe.h"

#include "QuadTree.h"

static constexpr int NUMBER_OF_THREADS = 16;

static constexpr GLfloat PARTICLE_MASS = 1.0e5;
static constexpr GLfloat BLACK_HOLE_MASS = 1.0e6;

float float_rand(float min, float max)
{
	float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
	return min + scale * (max - min);      /* [min, max] */
}

Universe::Universe( GLuint numberOfGalaxies, GLfloat galaxyRadius, GLuint numberOfStars )
	: _numberOfGalaxies(numberOfGalaxies)
	, _galaxyRadius(galaxyRadius)
	, _numberOfStars(numberOfStars)
	, _threadPool(NUMBER_OF_THREADS)
{

	_threadPool.init();

	srand(time(NULL));

	_galaxies.resize(_numberOfGalaxies);
	_stars.reserve(_numberOfGalaxies * _numberOfStars);

	for (auto& galaxy : _galaxies)
	{
		galaxy.radius = _galaxyRadius;
		//galaxy.center = { 0.0f, 0.0f, 0.0f };
		galaxy.center = { float_rand(-1,1), float_rand(-1,1), 0.0f };

		for (GLuint i = 0; i < _numberOfStars; i++)
		{
			float a = float_rand(0, 1) * 2 * M_PI;
			float r = galaxy.radius * sqrt(float_rand(0, 1));

			float displacementX = r * cos(a);
			float displacementY = r * sin(a);

			_stars.push_back({
				PARTICLE_MASS,
				{ galaxy.center.x + displacementX, galaxy.center.y + displacementY, 0.0f }
			});
		}

#ifdef ADD_BLACK_HOLE
		_stars.push_back({ BLACK_HOLE_MASS, {galaxy.center.x, galaxy.center.y, 0.0f} });
#endif
	}
}

void forceRoutine(QuadTreeNode* const root, Star* const star, int batchSize, float deltaTime)
{
	for (GLuint i = 0; i < batchSize; i++)
	{
		const glm::vec3& acc = root->calcTreeForce(&star[i]);
		star[i].velocity = star[i].velocity + glm::vec3(acc.x*deltaTime, acc.y*deltaTime, 0.0f);
		const glm::vec3& vel = star[i].velocity;
		star[i].position = star[i].position + glm::vec3(vel.x*deltaTime, vel.y*deltaTime, 0.0f);
	}
}

void Universe::step(float deltaTime)
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
	
	// Sending star batches
	std::vector<std::future<void>> futures;

	int batchSize = _stars.size() / NUMBER_OF_THREADS;
	for (GLuint i = 0; i < NUMBER_OF_THREADS; i++)
	{
		// Send last batch (bigger if size not divisible)
		if (i == NUMBER_OF_THREADS - 1)
		{
			futures.push_back(_threadPool.submit(
				forceRoutine,
				_root,
				_stars.data() + (i*batchSize),
				_stars.size()-((NUMBER_OF_THREADS-1)*batchSize),
				deltaTime
			));
		}
		else
		{
			futures.push_back(_threadPool.submit(
				forceRoutine,
				_root,
				_stars.data() + (i*batchSize),
				batchSize,
				deltaTime
			));
		}
	}


#ifdef DRAW_QUADTREE
	_quadTreeVerts.clear();
	_root->getQuadTreeVerts( _quadTreeVerts );
#endif

	// Make sure all threads are done
	for (auto& future : futures) future.get();

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
	_threadPool.shutdown();
}
