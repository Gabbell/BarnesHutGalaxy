#include "Transform.h"

Transform::Transform() {
	m_transform = glm::mat4(1.0f);
}

glm::mat4 Transform::getTransform() {
	return m_transform;
}

void Transform::addTransform(glm::mat4 transform) {
	m_transform *= transform;
}

void Transform::clearTransform() {
	m_transform = glm::mat4(1.0f);
}

Transform::~Transform() {
}
