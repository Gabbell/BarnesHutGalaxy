#pragma once

#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Universe.h"
#include "Mesh.h"

class Shader;

class UniverseSim
{
public:
	UniverseSim(GLuint width, GLuint height, GLfloat galaxyRadius, GLuint numberOfGalaxies, GLuint numberOfStars);
	~UniverseSim();

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

