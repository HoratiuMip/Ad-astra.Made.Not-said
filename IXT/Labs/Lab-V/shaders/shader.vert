#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColour;

out vec3 color;

uniform mat4 tmx_model;
uniform mat4 tmx_view;
uniform mat4 tmx_proj;

void main() {
    color = vertexColour;
    gl_Position = tmx_proj * tmx_view * tmx_model * vec4(vertexPosition, 1.0);
}

