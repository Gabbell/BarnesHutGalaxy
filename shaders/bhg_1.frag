#version 330 core

uniform uint numberOfStars;

flat in int vertexID;

void main() {
	if( uint(vertexID) < numberOfStars )
	{
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);
	}
} 