#pragma once

#include "CL/cl.h"

// Prints the QuadTree
//#define DRAW_QUADTREE

// Adds one supermassive star per galaxy at its center
//#define ADD_BLACK_HOLE

// To be passed to the GPU for the QuadTree
struct Node
{
	cl_float4 centerOfMass;
	cl_float mass;
	cl_int children[4];
	cl_int starIndex;
};

struct Star
{
	cl_float4 position;
	cl_float4 velocity;
	cl_float4 acceleration;
	cl_float mass;
	cl_int id;
};