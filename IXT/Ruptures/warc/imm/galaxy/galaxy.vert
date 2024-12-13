#version 410 core
//IXT#name<galaxy-vert>

layout( location = 0 ) in vec3 vrtx;
layout( location = 1 ) in vec3 nrm;
layout( location = 2 ) in vec2 tex_crd;

out VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    vs_out.tex_crd = tex_crd;
    vs_out.nrm = nrm;

    gl_Position = proj * view * model * vec4( vrtx, 1.0 );
}
