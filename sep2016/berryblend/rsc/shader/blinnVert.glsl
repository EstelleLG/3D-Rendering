#version 330
layout (location = 0) in vec3 vertposition;
layout (location = 1) in vec2 vertexuv;
layout (location = 2) in vec3 vertnormal;
out vec3 normal;
out vec2 uv;
out vec3 position;
uniform mat4 mvp;
uniform mat4 model;
void main (void) {
	gl_Position = mvp * vec4(vertposition, 1);
	uv = vertexuv;
	position = (model * vec4(vertposition, 1)).xyz;
	normal = normalize((model * vec4(vertnormal, 0)).xyz);
}