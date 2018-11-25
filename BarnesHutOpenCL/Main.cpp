#include "UniverseSim.h"

//==================================================================================================
// MAIN

int main()
{
	UniverseSim universeSim( 
		1000,					// WIDTH
		1000,					// HEIGHT
		GALAXYRADIUS,			// GALAXY RADIUS
		NUMBEROFGALAXIES,		// NUMBER OF GALAXIES 
		NUMBEROFSTARS			// NUMBER OF STARS PER GALAXY
	);

	universeSim.startLoop();

	return 0;
}