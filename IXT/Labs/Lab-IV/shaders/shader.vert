#version 410 core

layout( location = 0 ) in vec3 vrtx_pos;
layout( location = 1 ) in vec3 vrtx_rgb;

out vec3 rgb;

uniform vec2 vrtx_off;

void main() {
    rgb = vrtx_rgb;

    gl_Position = vec4( vrtx_pos + vec3( vrtx_off, .0 ), 1.0 );
}
