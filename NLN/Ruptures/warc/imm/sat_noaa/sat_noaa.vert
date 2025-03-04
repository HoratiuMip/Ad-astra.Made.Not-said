#version 410 core
//NLN#name<sat-noaa-vert>

layout( location = 0 ) in vec3 vrtx;
layout( location = 1 ) in vec3 nrm;
layout( location = 2 ) in vec2 tex_crd;

out VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
    vec3 sun_ray;
    vec3 lens_ray;
} vs_out;

uniform vec3 sun_pos;
uniform vec3 lens_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    vs_out.tex_crd  = tex_crd;
    vs_out.nrm      = nrm;
    vs_out.sun_ray  = sun_pos - vrtx;
    vs_out.lens_ray = lens_pos - vrtx;

    gl_Position = proj * view * model * vec4( vrtx, 1.0 );
}
