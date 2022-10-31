#version 330 core

in vec3 pos;
in vec4 colour;

out Vertex {
	vec4 colour;
}	OUT;

void main (void) {
	gl_Position = vec4 (pos, 1.0)
	OUT.colour = colour;
}