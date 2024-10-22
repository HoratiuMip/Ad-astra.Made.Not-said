#version 410 core

in vec3 rgb;

out vec4 frag_rgb;

void main() {
	frag_rgb = vec4( rgb, 1.0 );
}
