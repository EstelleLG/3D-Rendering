#version 330
out vec4 color; 
in vec2 uv;
uniform sampler2D tex;  //constant
void main(void) {
	color = texture(tex, uv);
}