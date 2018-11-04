#include "Galaxy.h"

#include "QuadTree.h"
#include "ThreadPool.h"

#include "glm/gtx/transform.hpp"
#include <future>

#ifdef TBB_IMP
#include "tbb/parallel_for_each.h"
#endif

static constexpr GLfloat PARTICLE_MASS = 1.0e4;
static constexpr GLfloat BLACK_HOLE_MASS = 1.0e8;
static constexpr GLfloat INITIAL_SPIN_FACTOR = 0.1f;

float float_rand(float min, float max)
{
	float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
	return min + scale * (max - min);      /* [min, max] */
}

Galaxy::Galaxy( GLfloat radius, glm::vec3 center )
	:_radius(radius), _center(center)
{
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

void Galaxy::setRandomCenter()
{
	_center = { float_rand(-1, 1), float_rand(-1, 1), 0.0f };
}

std::vector<glm::vec3> Galaxy::getVertices() const
{
	std::vector<glm::vec3> verts(_stars.size());

	for (GLuint i = 0; i < verts.size(); i++)
	{
		verts.at(i) = _stars.at(i).position;
	}

	return verts;
}
void Galaxy::initStars(GLuint numberOfStars)
{
	_stars.reserve(numberOfStars);

	for (GLuint i = 0; i < numberOfStars; i++)
	{
		float a = float_rand(0, 1) * 2 * M_PI;
		float r = _radius * sqrt(float_rand(0, 1));

		float displacementX = r * cos(a);
		float displacementY = r * sin(a);

		glm::mat4 spinMat(1.0f);
		glm::vec4 spinVec(displacementX, displacementY, 0.0f, 0.0f);
		spinMat = glm::rotate(spinMat, glm::radians(-90.0f), { 0.0f, 0.0f, 1.0f });
		spinVec = (_radius*(1/r)) * INITIAL_SPIN_FACTOR * (spinMat * spinVec);

		_stars.push_back({
			PARTICLE_MASS,
			{ _center.x + displacementX, _center.y + displacementY, 0.0f },
			spinVec
			});
	}

#ifdef ADD_BLACK_HOLE
	_stars.push_back({ BLACK_HOLE_MASS,{ _center.x, _center.y, 0.0f } });
#endif
}

void Galaxy::step(GLfloat deltaTime)
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

	// Building Quad Tree
	for (auto& star : _stars)
	{
		_root->insertStar(&star);
	}

	_root->computeMassDistribution();

	// Stepping stars

#ifdef MT_IMP
	// Sending star batches
	std::vector<std::future<void>> futures;

	int batchSize = _stars.size() / NUMBER_OF_THREADS;
	for (GLuint i = 0; i < NUMBER_OF_THREADS; i++)
	{
		// Send last batch (bigger if size not divisible)
		if (i == NUMBER_OF_THREADS - 1)
		{
			futures.push_back(ThreadPool::Instance().submit(
				forceRoutine,
				_root,
				_stars.data() + (i*batchSize),
				_stars.size() - ((NUMBER_OF_THREADS - 1)*batchSize),
				deltaTime
			));
		}
		else
		{
			futures.push_back(ThreadPool::Instance().submit(
				forceRoutine,
				_root,
				_stars.data() + (i*batchSize),
				batchSize,
				deltaTime
			));
		}
	}

	// Make sure all threads are done
	for (auto& future : futures) future.get();
#endif

#ifdef TBB_IMP
	tbb::parallel_for_each(_stars, [this, deltaTime](Star& star) {
		const glm::vec3& acc = _root->calcTreeForce(&star);
		star.velocity = star.velocity + glm::vec3(acc.x*deltaTime, acc.y*deltaTime, 0.0f);
		const glm::vec3& vel = star.velocity;
		star.position = star.position + glm::vec3(vel.x*deltaTime, vel.y*deltaTime, 0.0f);
	});
#endif

	delete _root;
}

#ifdef DRAW_QUADTREE
std::vector<glm::vec3> Galaxy::getQuadTreeVerts() const
{
	return _root->getQuadTreeVerts(_quadTreeVerts);
}
#endif
