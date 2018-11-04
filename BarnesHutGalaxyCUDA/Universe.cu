#include "Universe.h"

#include <ctime>

// TO REMOVE
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "cuda_runtime.h"
#include "vector_types.h"
#include "device_launch_parameters.h"

float float_rand(float min, float max)
{
	float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
	return min + scale * (max - min);      /* [min, max] */
}

//======================================
// Kernels
__device__ float3 sub(float3 a, float3 b)
{
	return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}
__device__ float length(float3 a)
{
	return sqrt((a.x*a.x)+(a.y*a.y)+(a.z*a.z));
}
__device__ float3 add(float3 a, float3 b)
{
	return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__global__ void calcForce(int numberOfStars, const Star* inStars, Star* outStars, float deltaTime)
{
	// Equivalent index in the array based on block size and thread
	const int index = (blockIdx.x * blockDim.x) + threadIdx.x;
	
	// Star that will be compared in this CUDA thread
	const Star& inputStar = inStars[index];

	float3 inPos = make_float3(inputStar.position.x, inputStar.position.y, 0.0f);

	float3 acc = make_float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < numberOfStars; i++)
	{
		if (i != index)
		{
			const Star& currStar = inStars[i];
			float3 curPos = make_float3(currStar.position.x, currStar.position.y, 0.0f);

			float3 difference = sub(curPos, inPos);
			float distance = length(difference) + SOFTENER;

			if (distance > 0)
			{
				float g = GRAVITATIONAL_CONSTANT * PARTICLE_MASS / (distance*distance*distance);
				acc.x += g * difference.x;
				acc.y += g * difference.y;
			}
		}
	}
	Star& outputStar = outStars[index];

	float3 starVel = make_float3(inputStar.velocity.x, inputStar.velocity.y, 0.0f);
	float3 outVel = add(starVel, make_float3(acc.x*deltaTime, acc.y*deltaTime, 0.0f));
	float3 outPos = add(inPos, make_float3(outVel.x*deltaTime, outVel.y*deltaTime, 0.0f));
	
	outputStar.position.x = outPos.x;
	outputStar.position.y = outPos.y;
	outputStar.velocity.x = outVel.x;
	outputStar.velocity.y = outVel.y;
}
//======================================

Universe::Universe( GLuint numberOfGalaxies, GLfloat galaxyRadius, GLuint numberOfStars )
	: _numberOfGalaxies(numberOfGalaxies)
	, _numberOfStars(numberOfStars)
{
	srand(time(NULL));

	_totalStars = _numberOfGalaxies * _numberOfStars;

	_galaxies.resize(_numberOfGalaxies);

	// Generating universe
	for (auto& galaxy : _galaxies)
	{

		galaxy.radius = galaxyRadius;
		galaxy.center = {float_rand(-1,1), float_rand(-1,1), 0.0f};
		//galaxy.center = { 0.0f,0.0f,0.0f };

		_stars.reserve(_numberOfStars);
		for (GLuint i = 0; i < _numberOfStars; i++)
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
				PARTICLE_MASS,
				{ galaxy.center.x + displacementX, galaxy.center.y + displacementY, 0.0f },
				spinVec,
				{0.0f,0.0f,0.0f}
				});
		}

#ifdef ADD_BLACK_HOLE
		_stars.push_back({ BLACK_HOLE_MASS,{ galaxy.center.x, galaxy.center.y, 0.0f } });
#endif
	}

	// Device prep
	cudaMalloc(&_dinStars, _totalStars * sizeof(Star));
	cudaMalloc(&_doutStars, _totalStars * sizeof(Star));

	cudaMemcpy(_dinStars, _stars.data(), _totalStars * sizeof(Star), cudaMemcpyHostToDevice);
	cudaMemcpy(_doutStars, _stars.data(), _totalStars * sizeof(Star), cudaMemcpyHostToDevice);
}

void Universe::step(float deltaTime)
{
	calcForce << < ceil(_totalStars /(float)BLOCKSIZE), BLOCKSIZE >> >(_totalStars, _dinStars, _doutStars, deltaTime);

	cudaMemcpy(_stars.data(), _doutStars, _totalStars * sizeof(Star), cudaMemcpyDeviceToHost);
	cudaMemcpy(_dinStars, _doutStars, _totalStars * sizeof(Star), cudaMemcpyDeviceToDevice);
}

std::vector<glm::vec3> Universe::getVertices() const
{
	std::vector<glm::vec3> verts;
	verts.reserve(_numberOfStars);
	for (auto& star : _stars)
	{
		verts.push_back(star.position);
	}
	return verts;
}

Universe::~Universe()
{
	cudaFree(_dinStars);
	cudaFree(_doutStars);
}
