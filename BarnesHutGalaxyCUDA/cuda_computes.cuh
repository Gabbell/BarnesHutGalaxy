#include "Common.h"

__global__ void computeForce(Star* stars, Node* tree, float cellRadius, int numberOfStars);
__global__ void integrate(Star* stars, int numberOfStars, float deltaTime);