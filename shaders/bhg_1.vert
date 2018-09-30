#version 330 core
  
layout (location = 0) in vec3 position;

flat out int vertexID;

void main() {
	vertexID = gl_VertexID;
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
}