#include "UniverseSim.h"

#include <iostream>

#include "cuda_runtime.h"
//==================================================================================================
// MAIN

int main()
{
	UniverseSim universeSim( 
		1000,					// WIDTH
		1000,					// HEIGHT
		0.3,					// GALAXY RADIUS
		2,						// NUMBER OF GALAXIES 
		10000					// NUMBER OF STARS PER GALAXY
	);

	universeSim.startLoop();

	return 0;
}