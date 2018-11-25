typedef struct Star
{
	float4 position;
	float4 velocity;
	float4 acceleration;
	float mass;
	int id;
} __attribute__((packed)) StarStruct;

__kernel void integrate(__global StarStruct* stars, int numberOfStars, float deltaTime)
{
	const int starIdx = get_global_id(0);
	if (starIdx < numberOfStars)
	{
		stars[starIdx].velocity += stars[starIdx].acceleration * deltaTime;
		stars[starIdx].position += stars[starIdx].velocity * deltaTime;
	}
}