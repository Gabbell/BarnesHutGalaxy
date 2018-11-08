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
		1.0,					// GALAXY RADIUS
		1,						// NUMBER OF GALAXIES 
		5000					// NUMBER OF STARS PER GALAXY
	);

	universeSim.startLoop();

	return 0;
}