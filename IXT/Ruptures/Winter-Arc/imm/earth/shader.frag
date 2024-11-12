#version 410 core

in vec3 colour;
in vec2 pass_texture;

out vec4 fragmentColour;

uniform sampler2D diffuse_texture;

void main() {
    fragmentColour = texture( diffuse_texture, pass_texture );
}