#pragma once

#include "Universe.h"
#include "Mesh.h"
#include "Constants.h"

#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"


class Shader;

class UniverseSim
{
public:
	UniverseSim(uint32 width, uint32 height, float32 galaxyRadius, uint32 numberOfGalaxies, uint32 numberOfStars);
	~UniverseSim();

	void startLoop();

private:
	void render(float32 delta);

	Universe _universe;

	// Windowing
	GLFWwindow* _window;
	int _width;
	int _height;

	// Special Stuff
	Shader* _galaxyShader;
};

