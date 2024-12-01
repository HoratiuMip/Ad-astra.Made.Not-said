#version 410 core

layout( location = 0 ) in vec3 vrtx;
layout( location = 1 ) in vec3 nrm;
layout( location = 2 ) in vec2 tex_crd;

out VS_OUT {
    vec2      tex_crd;
    vec3      nrm;
    vec3      sun_ray;
    flat vec3 lens;
    flat mat4 pvm;
} vs_out;

uniform vec3 sun_pos;
uniform vec3 lens_pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    vs_out.tex_crd = tex_crd;
    vs_out.nrm     = nrm;
    vs_out.sun_ray = sun_pos - vrtx;
    vs_out.lens    = lens_pos;
    vs_out.pvm     = proj * view * model;

    gl_Position = vec4( vrtx, 1.0 );
}
