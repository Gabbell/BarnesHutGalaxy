#define WARPSIZE 32
#define MAXDEPTH 64
#define BLOCKSIZE 128
#define GRAVITATIONAL_CONSTANT 6.67408e-11
#define THETA 100.0f
#define SOFTENER 1.0f

typedef struct Node
{
	float4 centerOfMass;
	float mass;
	int children[4];
	int starIndex;
}__attribute__((packed)) NodeStruct;

typedef struct Star
{
	float4 position;
	float4 velocity;
	float4 acceleration;
	float mass;
	int id;
} __attribute__((packed)) StarStruct;

__kernel void computeForce(__global StarStruct* stars, __global NodeStruct* tree, float cellRadius, int numberOfStars)
{
	// One stack per thread block
	__local volatile float depth[MAXDEPTH * BLOCKSIZE / WARPSIZE];
	__local volatile int stack[MAXDEPTH * BLOCKSIZE / WARPSIZE];

	// Equivalent index in the array based on block size and current work thread
	const int warpGroupIdx = get_local_id(0) / WARPSIZE;
	const int warpIdx = get_local_id(0) % WARPSIZE;
	const int stride = get_global_size(0);
	const int stackStartIdx = MAXDEPTH * warpGroupIdx;
	int starIdx = get_global_id(0);

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
		float4 pos = stars[starIdx].position;
		float4 acc = { 0.0f, 0.0f, 0.0f, 0.0f };

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
		barrier(CLK_LOCAL_MEM_FENCE);

		// While this warp's stack is not empty, perform the acceleration calculations
		int top = stackOffset + stackStartIdx;
		while (top >= stackStartIdx)
		{
			int nodeIdx = stack[top];
			float nextQuadrantSize = 0.25f*depth[top];
			for (int i = 0; i < 4; i++)
			{
				int childIdx = tree[nodeIdx].children[i];

				if (childIdx > -1)
				{
					float4 com = tree[childIdx].centerOfMass;
					float4 difference = com - pos;
					float squaredDist = dot(difference, difference) + SOFTENER;

					// Compute acceleration if leaf node or meets the criterion
					if (tree[childIdx].starIndex != -1 || work_group_all(nextQuadrantSize <= squaredDist))
					{
						float inverseDist = rsqrt(squaredDist);
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
		barrier(CLK_LOCAL_MEM_FENCE);
	}
}