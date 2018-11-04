#include "cuda_runtime.h"

#include <iostream>

#include "UniverseSim.h"

//==================================================================================================
// MAIN

int main()
{
	UniverseSim universeSim( 
		1000,					// WIDTH
		1000,					// HEIGHT
		0.5,					// GALAXY RADIUS
		2,						// NUMBER OF GALAXIES 
		5000					// NUMBER OF STARS PER GALAXY
	);

	universeSim.startLoop();

	return 0;
}