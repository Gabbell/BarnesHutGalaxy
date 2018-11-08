#pragma once

#include "Common.h"
#include "Types.h"

struct Galaxy
{
	Galaxy() = default;
	Galaxy(float32 radius, float3 center);
	~Galaxy() = default;

	float32 radius;
	float3 center;
	float3 colour;
};

