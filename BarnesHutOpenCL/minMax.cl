typedef struct Star
{
	float4 position;
	float4 velocity;
	float4 acceleration;
	float mass;
	int id;
} __attribute__((packed)) StarStruct;

__kernel void minMax(__global StarStruct* stars, __global float4* min, __global float4* max, int numberOfStars)
{
	int idx = 0;
	while (idx < numberOfStars)
	{
		if (stars[idx].position.x < min->x)
		{
			min->x = stars[idx].position.x;
		}
		if (stars[idx].position.y < min->y)
		{
			min->y = stars[idx].position.y;
		}
		if (stars[idx].position.x > max->x)
		{
			max->x = stars[idx].position.x;
		}
		if (stars[idx].position.y > max->y)
		{
			max->y = stars[idx].position.y;
		}

		idx++;
	}
}