#pragma once
#include "Constants.h"
#include "Types.h"

#include "vector_types.h"

// Prints the QuadTree
//#define DRAW_QUADTREE

// Adds one supermassive star per galaxy at its center
//#define ADD_BLACK_HOLE

// To be passed to the GPU for the QuadTree
struct Node
{
	int starIndex;
	float mass;
	float3 centerOfMass;
	int children[4];
};

struct Star
{
	uint32 id;
	float mass;
	float3 position;
	float3 velocity;
	float3 acceleration;
};
