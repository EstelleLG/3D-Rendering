#version 330

layout (location = 0) in vec3 vertposition;
uniform mat4 mvp;
void main (void) {
	gl_Position = mvp * vec4(vertposition, 1);
}