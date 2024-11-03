#version 410 core

in vec3 color;

out vec4 fragmentColour;


void main() {
    fragmentColour = vec4(color, 1.0);
}
