#include <iostream>
#include <array>

#include "glm/glm.hpp"

#include "UniverseSim.h"
#include "Shader.h"
#include "Mesh.h"

#define PRIMITIVE_RESTART 0xFFFFFFFF

//==================================================================================================
// CALLBACKS

void key_callback(GLFWwindow* _window, int key, int scancode, int action, int mode)
{
	//Exit when escape is pressed
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(_window, true);
}

void mouse_button_callback(GLFWwindow* _window, int button, int action, int mods)
{

}

void mouse_callback(GLFWwindow* _window, double xpos, double ypos)
{

}

void window_size_callback(GLFWwindow* _window, int width, int height)
{
	width = width;
	height = height;
	glViewport(0, 0, width, height);
}

//==================================================================================================
// CLASS FUNCTIONS

UniverseSim::UniverseSim(GLuint width, GLuint height, GLfloat galaxyRadius, GLuint numberOfGalaxies, GLuint numberOfStars )
	: _width(width), _height(height), _universe(numberOfGalaxies, galaxyRadius, numberOfStars)
{
	// Init GLFW
	glfwInit();

	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	std::cout << "Status: Using GLFW " << glfwGetVersionString() << std::endl;

	// GLFW init
	_window = glfwCreateWindow(width, height, "Barnes-Hut Galaxy Simulator 1", nullptr, nullptr);
	if (_window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(_window);

	// GLEW init
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
	}

	std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "Status: Using OpenGL " << glGetString(GL_VERSION) << std::endl;

	// Define the viewport dimensions
	glfwGetFramebufferSize(_window, &_width, &_height);
	glViewport(0, 0, width, height);

	// Set the required callback functions
	glfwSetKeyCallback(_window, key_callback);
	glfwSetCursorPosCallback(_window, mouse_callback);
	glfwSetMouseButtonCallback(_window, mouse_button_callback);
	glfwSetWindowSizeCallback(_window, window_size_callback);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(PRIMITIVE_RESTART);

	_galaxyShader = new Shader("../shaders/bhg_1.vert", "../shaders/bhg_1.frag");
	_galaxyShader->use();

	_galaxyShader->setUint( "numberOfStars", numberOfStars);
}

void UniverseSim::startLoop()
{
	// Game Loop
	GLuint frames = 0;
	GLfloat counter = 0;
	GLfloat delta = 0;
	GLfloat currentTime = 0;

	while (!glfwWindowShouldClose(_window))
	{
		currentTime = glfwGetTime();

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		render(delta);

		// Swap the screen buffers
		glfwSwapBuffers(_window);

		delta = glfwGetTime() - currentTime;

		counter += delta;
		if (counter >= 1) {
			counter = 0;
			std::cout << "FPS: " << frames << std::endl;
			frames = 0;
		}
		else {
			frames++;
		}
	}
}

void UniverseSim::render(GLfloat delta)
{
	glClear(GL_COLOR_BUFFER_BIT);

	// Solve one delta time for universe
	_universe.step(delta);

	// Building Universe Mesh
	Mesh universeMesh( _universe.getVertices() );
	glBindVertexArray(universeMesh.getVAO());

	glDrawArrays(GL_POINTS, 0, universeMesh.getVerts().size());
	glBindVertexArray(0);

#ifdef DRAW_QUADTREE
	std::vector<GLuint> indices;
	std::vector<glm::vec3> vertices = _universe.getQuadTreeVerts();

	// Indices generation
	for (GLuint i = 0; i < vertices.size(); i += 4)
	{
		indices.push_back(i);
		indices.push_back(i+1);
		indices.push_back(i+1);
		indices.push_back(i+2);
		indices.push_back(i+2);
		indices.push_back(i+3);
		indices.push_back(i+3);
		indices.push_back(i);
		indices.push_back(PRIMITIVE_RESTART);
	}

	Mesh quadTreeMesh( vertices, indices);
	glBindVertexArray(quadTreeMesh.getVAO());
	glDrawElements(GL_LINE_STRIP, indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
#endif
}

UniverseSim::~UniverseSim()
{
	delete _galaxyShader;
}
