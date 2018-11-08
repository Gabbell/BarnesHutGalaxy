#include <iostream>

#include "UniverseSim.h"

//==================================================================================================
// MAIN

int main()
{
	UniverseSim universeSim( 
		1000,					// WIDTH
		1000,					// HEIGHT
		0.9,					// GALAXY RADIUS
		1,						// NUMBER OF GALAXIES 
		10000					// NUMBER OF STARS PER GALAXY
	);

	universeSim.startLoop();

	return 0;
}