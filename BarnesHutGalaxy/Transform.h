#pragma once

#include "glm/glm.hpp"
class Transform {
public:
	Transform();

	glm::mat4 getTransform();

	void addTransform(glm::mat4 transform);
	void clearTransform();

	~Transform();

private:
	glm::mat4 m_transform;
};

