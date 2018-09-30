#pragma once

#include <vector>

#include "Universe.h"
#include "Mesh.h"

class Shader;

class UniverseDisplay
{
public:
	UniverseDisplay(GLuint width, GLuint height, GLuint numberOfGalaxies, GLuint numberOfStars);
	~UniverseDisplay();

	void startLoop();

private:
	void render(GLfloat delta);

	Universe _universe;

	// Windowing
	GLFWwindow* _window;
	int _width;
	int _height;

	// Special Stuff
	Shader* _galaxyShader;
};

