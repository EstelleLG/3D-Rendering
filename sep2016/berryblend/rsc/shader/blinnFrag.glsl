#version 330
in vec3 normal;
in vec3 position;
out vec3 fragColor;
uniform vec3 camPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform int shininess;
uniform vec3 color;
uniform sampler2DShadow shadowMap;
uniform mat4 shadowMVP;

void main(void) {
	
	vec3 viewDir = normalize(camPos - position);
	vec3 halfVec = normalize(viewDir - lightDir);
	float spec = pow(max(dot(normal, halfVec), 0.0), shininess);
	vec3 specular = lightColor * spec;
	
	vec3 diffuse = lightColor * dot(-lightDir, normal);
	fragColor = color * specular + color * diffuse;
	
	vec4 shadowPos4 = shadowMVP * vec4(position, 1);
	vec3 shadowPos = shadowPos4.xyz/shadowPos4.w;

	shadowPos.z -= 0.001;
	float shadow = texture(shadowMap, shadowPos);
	fragColor = shadow * fragColor;
	fragColor = vec3(1, 0, 0);
	
}