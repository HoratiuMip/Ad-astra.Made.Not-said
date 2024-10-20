#version 410 core

out vec4 frag_color;

void main() {
	vec3 color = vec3(1.0f, 0.0f, 0.0f);
	frag_color = vec4 (color, 1.0);
}
