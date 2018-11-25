#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include <string>

class Shader {
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	Shader();
	~Shader();

	void use();
	void setFloat(const std::string &name, GLfloat value) const;
	void setVec4(const std::string &name, const glm::vec4 &value) const;
	void setMat3(const std::string &name, const glm::mat3 &mat) const;
	void setMat4(const std::string &name, const glm::mat4 &mat) const;
	void setUint(const std::string &name, const GLuint &value) const;

private:
	GLuint m_id;
};

