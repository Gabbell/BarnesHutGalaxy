#include <iostream>
#include <random>
#include <functional>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "UniverseDisplay.h"

//==================================================================================================
// MAIN
int main()
{
	UniverseDisplay universeDisplay( 1000, 1000, 2, 250);

	universeDisplay.startLoop();

	return 0;
}