#include "Universe.h"
#include "QuadTree.h"
#include "cuda_computes.cuh"

#include <ctime>

#include "glm/gtx/transform.hpp"
#include "cuda_runtime.h"

float float_rand(float min, float max)
{
	float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
	return min + scale * (max - min);      /* [min, max] */
}

Universe::Universe(uint32 numberOfGalaxies, float32 galaxyRadius, uint32 numberOfStars)
	: _numberOfGalaxies(numberOfGalaxies)
	, _numberOfStars(numberOfStars)
	, _totalStars(_numberOfGalaxies * _numberOfStars)
	, _numberOfNodes(4 * _totalStars)
{
	srand(time(NULL));

	_galaxies.resize(_numberOfGalaxies);

	// Generating universe
	uint32 starIndex = 0;
	for (auto& galaxy : _galaxies)
	{
		galaxy.radius = galaxyRadius;
		//galaxy.center = {float_rand(-1,1), float_rand(-1,1), 0.0f};
		galaxy.center = { 0.0f,0.0f,0.0f };

		_stars.reserve(_numberOfStars);
		for (uint32 i = 0; i < _numberOfStars; i++)
		{
			float a = float_rand(0, 1) * 2 * M_PI;
			float r = galaxy.radius * sqrt(float_rand(0, 1));

			float displacementX = r * cos(a);
			float displacementY = r * sin(a);

			glm::mat4 spinMat(1.0f);
			glm::vec4 spinVec(displacementX, displacementY, 0.0f, 0.0f);
			spinMat = glm::rotate(spinMat, glm::radians(-90.0f), { 0.0f, 0.0f, 1.0f });
			spinVec = INITIAL_SPIN_FACTOR * (spinMat * spinVec);

			_stars.push_back({
				starIndex++,
				PARTICLE_MASS,
				make_float3(galaxy.center.x + displacementX, galaxy.center.y + displacementY, 0.0f ),
				make_float3(spinVec.x, spinVec.y, spinVec.z),
				make_float3(0.0f,0.0f,0.0f)
				});
		}

#ifdef ADD_BLACK_HOLE
		_stars.push_back({ BLACK_HOLE_MASS,{ galaxy.center.x, galaxy.center.y, 0.0f } });
#endif
	}

	// Device Prep
	cudaMalloc(&_dStars, _totalStars * sizeof(Star));
	cudaMalloc(&_dNodes, _numberOfNodes * sizeof(Node));
	cudaMemcpy(_dStars, _stars.data(), _totalStars * sizeof(Star), cudaMemcpyHostToDevice);
}

void Universe::step(float deltaTime)
{
	cudaDeviceSynchronize();

	float3 min = make_float3(FLT_MAX, FLT_MAX, 0.0);
	float3 max = make_float3(-FLT_MAX, -FLT_MAX, 0.0);


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
	float maxCellRadius = 0.5f*(max.x - min.x);

	// Init Quad Tree
	_root = new QuadTreeNode(0, min, max, nullptr);
	for (auto& star : _stars)
	{
		_root->insertStar(&star);
	}

	// Computing Mass Distribution
	_root->computeMassDistribution();

	// Traversing and populating the array that will go to GPU
	std::vector<Node> gpuQuadTree(_numberOfNodes);
	QuadTreeNode* currentNode = _root;
	_root->traverseAndPopulate(gpuQuadTree.data());

	// Upload the CPU made tree to the GPU
	cudaMemcpy(_dNodes, gpuQuadTree.data(), _numberOfNodes * sizeof(Node), cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();

	// Launching kernels
	computeForce <<< ceil(_totalStars /(float)BLOCKSIZE), BLOCKSIZE >>>
		(_dStars, _dNodes, maxCellRadius, _totalStars);

	integrate <<< ceil(_totalStars / (float)BLOCKSIZE), BLOCKSIZE >> >
		(_dStars, _totalStars, deltaTime);

	// Downloading stars data back from the GPU
	// We keep the most up to date stars locally on the device
	cudaMemcpy(_stars.data(), _dStars, _totalStars * sizeof(Star), cudaMemcpyDeviceToHost);

	QuadTreeNode::s_idCounter = 0;
}

std::vector<glm::vec3> Universe::getVertices() const
{
	std::vector<glm::vec3> verts;
	verts.reserve(_numberOfStars);
	for (auto& star : _stars)
	{
		float3 pos = star.position;
		verts.push_back({ pos.x, pos.y, pos.z });
	}
	return verts;
}

Universe::~Universe()
{
	cudaFree(_dStars);
	cudaFree(_dNodes);
}
