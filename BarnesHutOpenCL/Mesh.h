#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "GL/glew.h"

class Mesh {
public:
	//Mesh(const char* objectName);
	Mesh(std::vector<glm::vec3> vertices);
	Mesh(std::vector<glm::vec3> vertices, std::vector<GLuint> indices);
	Mesh();
	~Mesh();

	std::vector<glm::vec3>& getVerts() { return m_vertices; }
	std::vector<GLuint>& getIndices() { return m_indices; }
	std::vector<glm::vec3>& getNorms() { return m_normals; }
	std::vector<glm::vec2>& getUvs() { return m_uvs; }
	std::vector<glm::vec3>& getPositions() { return m_positions; }
	GLuint getVBO() const { return m_VBO; }
	GLuint getEBO() const { return m_EBO; }
	GLuint getNormsVBO() const { return m_normsVBO; }
	GLuint getVAO() const { return m_VAO; }

	void setVerts(const std::vector<glm::vec3>& vertices) { m_vertices = vertices; }
	void setIndices(const std::vector<GLuint>& indices) { m_indices = indices; }
	void setNorms(const std::vector<glm::vec3>& normals) { m_normals = normals; }
	void setUvs(const std::vector<glm::vec2>& uvs) { m_uvs = uvs; }
	void setPositions(const std::vector<glm::vec3>& positions) { m_positions = positions; }
	void setPosition(const glm::vec3& position) { m_positions[0] = position; }
	void setPosition(const GLuint& index, const glm::vec3& position) { m_positions[index] = position; }

	void addPosition(glm::vec3 position) { m_positions.push_back(position); }
	void removePosition(GLuint index) { m_positions.erase(m_positions.begin() + index); }
	void translate(glm::vec3 translation) { m_positions[0] += translation; }
	void clearPositions() { m_positions.clear(); }

private:
	std::vector<glm::vec3> m_vertices;
	std::vector<GLuint> m_indices;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_uvs;
	std::vector<glm::vec3> m_positions;

	//For now
	GLuint m_VAO;
	GLuint m_EBO;
	GLuint m_VBO;
	GLuint m_normsVBO;
};

