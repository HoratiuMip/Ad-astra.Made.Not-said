#version 410 core
//NLN#name<barracuda-ctrl-vert>

layout( location = 0 ) in vec3 vrtx;
layout( location = 1 ) in vec3 nrm;
layout( location = 2 ) in vec2 tex_crd;

out VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
    vec3 lens2vrtx;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 lens_pos;

void main() {
    vs_out.tex_crd   = tex_crd;
    vs_out.nrm       = ( model * vec4( nrm, 0.0 ) ).xyz;

    gl_Position = model * vec4( vrtx, 1.0 );

    vs_out.lens2vrtx = vec3( gl_Position ) - lens_pos;

    gl_Position = proj * view * gl_Position;
}
