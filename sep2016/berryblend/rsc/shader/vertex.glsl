#version 330
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 vertexuv;
out vec2 uv;
uniform mat4 mvp;
void main (void) {
	gl_Position = mvp * vec4(position, 1);
	uv = vertexuv;
}
