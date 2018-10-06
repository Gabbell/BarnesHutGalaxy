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
		1,						// NUMBER OF GALAXIES 
		2500					// NUMBER OF STARS PER GALAXY
	);

	universeSim.startLoop();

	return 0;
}