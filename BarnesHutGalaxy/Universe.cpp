#include "QuadTree.h"
#include "Universe.h"

#include <ctime>

#include "glm/glm.hpp"

#ifdef MT_IMP
#include "ThreadPool.h"
#endif
#ifdef TBB_IMP
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for_each.h"
#endif

Universe::Universe( GLuint numberOfGalaxies, GLfloat galaxyRadius, GLuint numberOfStars )
	: _numberOfGalaxies(numberOfGalaxies)
{

#ifdef TBB_IMP
	tbb::task_scheduler_init init(NUMBER_OF_THREADS);
#endif

#ifdef MT_IMP
	ThreadPool::Instance().init(NUMBER_OF_THREADS);
#endif

	srand(time(NULL));

	_galaxies.resize(_numberOfGalaxies);

	// Generating galaxies
	for (auto& galaxy : _galaxies)
	{

		galaxy.setRadius(galaxyRadius);
		galaxy.setCenter({0.0f,0.0f,0.0f});
		//galaxy.setRandomCenter();

		galaxy.initStars(numberOfStars);
	}
}

void Universe::step(float deltaTime)
{
#ifdef MT_IMP
	for (auto& galaxy : _galaxies)
	{
		galaxy.step(deltaTime);
	}
#endif

#ifdef TBB_IMP
	tbb::parallel_for_each(_galaxies, [deltaTime](Galaxy& galaxy) {
		galaxy.step(deltaTime);
});
#endif
}

std::vector<glm::vec3> Universe::getVertices() const
{
	std::vector<glm::vec3> verts;
	for (auto& galaxy : _galaxies)
	{
		std::vector<glm::vec3> galaxyVerts = galaxy.getVertices();
		verts.insert(verts.end(), std::make_move_iterator(galaxyVerts.begin()), std::make_move_iterator(galaxyVerts.end()) );
	}
	return verts;
}

Universe::~Universe()
{
}
