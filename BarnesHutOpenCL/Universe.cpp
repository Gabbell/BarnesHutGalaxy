#include "Universe.h"
#include "QuadTree.h"
#include "Constants.h"

#include <ctime>
#include <iostream>

#include "glm/gtx/transform.hpp"

float float_rand(float min, float max)
{
	float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
	return min + scale * (max - min);      /* [min, max] */
}

Universe::Universe(uint32 numberOfGalaxies, float32 galaxyRadius, uint32 numberOfStars)
	: _numberOfGalaxies(numberOfGalaxies)
	, _numberOfStars(numberOfStars)
	, _totalStars(_numberOfGalaxies * _numberOfStars)
{
	srand(time(NULL));

	_galaxies.resize(_numberOfGalaxies);

	// Generating universe
	int starIndex = 0;
	_stars.reserve(_totalStars);

	for (auto& galaxy : _galaxies)
	{
		galaxy.radius = galaxyRadius;
		galaxy.center = {float_rand(-1,1), float_rand(-1,1), 0.0f};
		//galaxy.center = { 0.0f,0.0f,0.0f };
		
		for (uint32 i = 0; i < _numberOfStars; i++)
		{
			float a = float_rand(0, 1) * 2 * M_PI;
			float r = galaxy.radius * sqrt(float_rand(0, 1));

			float displacementX = r * cos(a);
			float displacementY = r * sin(a);

			glm::mat4 spinMat(1.0f);
			glm::vec4 spinVec(displacementX, displacementY, 0.0f, 0.0f);
			spinMat = glm::rotate(spinMat, glm::radians(-90.0f), { 0.0f, 0.0f, 1.0f });
			spinVec = INITIAL_SPIN_FACTOR * glm::normalize(spinMat * spinVec);

			_stars.push_back({
				{galaxy.center.x + displacementX, galaxy.center.y + displacementY, 0.0f, 0.0f},
				{spinVec.x, spinVec.y, spinVec.z},
				{0.0f, 0.0f, 0.0f},
				PARTICLE_MASS,
				starIndex++
			});
		}

#ifdef ADD_BLACK_HOLE
		_stars.push_back({
			{galaxy.center.x, galaxy.center.y, 0.0f },
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f},
			BLACK_HOLE_MASS,
			starIndex++,
			});
#endif
	}

	_oclh.allocMem();
	_oclh.uploadStarsGPU(_stars.data());

	// For cl_mems
	_oclh.initKernelMem();

	// For constant scalars
	_oclh.setComputeArg(3, sizeof(cl_int), _totalStars);
	_oclh.setIntegrateArg(1, sizeof(cl_int), _totalStars);
	_oclh.setCpuArg(3, sizeof(cl_int), _totalStars);
}

void Universe::step(float32 deltaTime)
{
	cl_float3 min = { FLT_MAX, FLT_MAX, 0.0 };
	cl_float3 max = { -FLT_MAX, -FLT_MAX, 0.0 };

	_oclh.uploadStarsCPU(_stars.data());
	_oclh.minMax(&min, &max);

	float32 maxCellRadius = 0.5f*(max.x - min.x);

	// Init Quad Tree
	_root = new QuadTreeNode(0, { min.x,min.y,min.z }, { max.x,max.y,max.z }, nullptr);
	for (auto& star : _stars)
	{
		_root->insertStar(&star);
	}

#ifdef DRAW_QUADTREE
	_quadTreeVerts.clear();
	_root->getQuadTreeVerts(_quadTreeVerts);
#endif

	// Computing Mass Distribution
	_root->computeMassDistribution();

	// Traversing and populating the array that will go to GPU
	std::vector<Node> gpuQuadTree(NUMBEROFNODES);
	_root->traverseAndPopulate(gpuQuadTree.data());

	_oclh.setComputeArg(2, sizeof(float32), maxCellRadius);
	_oclh.setIntegrateArg(2, sizeof(float32), deltaTime);

	_oclh.uploadStepData(gpuQuadTree.data());

	// Launching kernels and downloading stars back to the host
	_oclh.computeForceAndIntegrate(_stars.data());

	QuadTreeNode::s_idCounter = 0;
	delete _root;
}

std::vector<glm::vec3> Universe::getVertices() const
{
	std::vector<glm::vec3> verts;
	verts.reserve(_totalStars);
	for (auto& star : _stars)
	{
		cl_float3 starPos = star.position;
		glm::vec3 pos = { starPos.x, starPos.y, starPos.z };
		verts.push_back({ pos.x, pos.y, pos.z });
	}
	return verts;
}

Universe::~Universe()
{
	//cudaFree(_dStars);
	//cudaFree(_dNodes);
}
