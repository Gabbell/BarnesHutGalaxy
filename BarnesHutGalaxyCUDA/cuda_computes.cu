#include "cuda_computes.cuh"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "helper_math.h"
#include <cmath>

__global__ void computeForce(Star* stars, Node* tree, float cellRadius, int numberOfStars)
{
	// One stack per thread block
	__shared__ float depth[(MAXDEPTH * BLOCKSIZE) / WARPSIZE];
	__shared__ int stack[(MAXDEPTH * BLOCKSIZE) / WARPSIZE];

	// Equivalent index in the array based on block size and current work thread
	const int warpGroupIdx = threadIdx.x / warpSize;
	const int warpIdx = threadIdx.x % warpSize;
	const int stride = blockDim.x * gridDim.x;
	const int stackStartIdx = MAXDEPTH * warpGroupIdx;
	int starIdx = (blockIdx.x * blockDim.x) + threadIdx.x;

	// Getting the number of valid children from the root node
	int stackOffset = -1;
	for (int i = 0; i < 4; i++)
	{
		if (tree[0].children[i] != -1)
		{
			stackOffset++;
		}
	}

	// Compute acceleration for every star in the block
	while (starIdx < numberOfStars)
	{
		float3 pos = stars[starIdx].position;
		float3 acc = make_float3(0.0f);

		// Initialize the stack since we are on the first thread on the warp
		if (warpIdx == 0)
		{
			for (int i = 0; i < 4; i++)
			{
				int childIdx = 0;
				if (tree[0].children[i] != -1)
				{
					stack[stackStartIdx + childIdx] = tree[0].children[i];
					depth[stackStartIdx + childIdx] = cellRadius * cellRadius / THETA;
					childIdx++;
				}
			}
		}
		// Making sure all threads on the warp have the updated stack
		__syncthreads();

		// While this warp's stack is not empty, perform the acceleration calculations
		int top = stackOffset + stackStartIdx;
		while (top >= stackStartIdx)
		{
			int nodeIdx = stack[top];
			float nextQuadrantSize = 0.25*depth[top];
			for (int i = 0; i < 4; i++)
			{
				int childIdx = tree[nodeIdx].children[i];

				if (childIdx > -1)
				{
					float3 difference = tree[childIdx].centerOfMass - pos;
					double squaredDist = dot(difference, difference) + SOFTENER;

					// Compute acceleration if leaf node or meets the criterion
					if (tree[childIdx].starIndex != -1 || __all(nextQuadrantSize <= squaredDist))
					{
						double inverseDist = rsqrtf(squaredDist);
						float g = tree[childIdx].mass * inverseDist * inverseDist * inverseDist * GRAVITATIONAL_CONSTANT;
						acc += g * difference;
					}
					else
					{
						if (warpIdx == 0)
						{
							stack[top] = childIdx;
							depth[top] = nextQuadrantSize;
						}
						top++;
					}
				}
			}
			top--;
		}
		// Updating the star's acceleration
		stars[starIdx].acceleration = acc;

		// Jumping one grid to make sure we're not working on the same star as another thread
		starIdx += stride;

		// Make sure that the next iteration of the loop has updated values
		__syncthreads();
	}
}

__global__ void integrate(Star* stars, int numberOfStars, float deltaTime)
{
	const int starIdx = (blockIdx.x * blockDim.x) + threadIdx.x;
	if (starIdx < numberOfStars)
	{
		stars[starIdx].velocity += stars[starIdx].acceleration * deltaTime;
		stars[starIdx].position += stars[starIdx].velocity * deltaTime;
	}
}

//__global__ void calcForce(int numberOfStars, const Star* inStars, Star* outStars, float deltaTime)
//{
//	// Equivalent index in the array based on block size and thread
//	const int index = (blockIdx.x * blockDim.x) + threadIdx.x;
//
//	// Star that will be compared in this CUDA thread
//	const Star& inputStar = inStars[index];
//
//	float3 inPos = make_float3(inputStar.position.x, inputStar.position.y, 0.0f);
//
//	float3 acc = make_float3(0.0f, 0.0f, 0.0f);
//	for (int i = 0; i < numberOfStars; i++)
//	{
//		if (i != index)
//		{
//			const Star& currStar = inStars[i];
//			float3 curPos = make_float3(currStar.position.x, currStar.position.y, 0.0f);
//
//			float3 difference = sub(curPos, inPos);
//			float distance = length(difference) + SOFTENER;
//
//			if (distance > 0)
//			{
//				float g = GRAVITATIONAL_CONSTANT * PARTICLE_MASS / (distance*distance*distance);
//				acc.x += g * difference.x;
//				acc.y += g * difference.y;
//			}
//		}
//	}
//	Star& outputStar = outStars[index];
//
//	float3 starVel = make_float3(inputStar.velocity.x, inputStar.velocity.y, 0.0f);
//	float3 outVel = add(starVel, make_float3(acc.x*deltaTime, acc.y*deltaTime, 0.0f));
//	float3 outPos = add(inPos, make_float3(outVel.x*deltaTime, outVel.y*deltaTime, 0.0f));
//
//	outputStar.position.x = outPos.x;
//	outputStar.position.y = outPos.y;
//	outputStar.velocity.x = outVel.x;
//	outputStar.velocity.y = outVel.y;
//}