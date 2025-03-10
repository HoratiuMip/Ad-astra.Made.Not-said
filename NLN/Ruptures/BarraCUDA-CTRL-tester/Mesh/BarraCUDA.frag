#version 410 core
//NLN#name<barracuda-ctrl-frag>

in VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
    vec3 lens2vrtx;
} vs_out;

out vec4 final;

uniform vec3 Kd;

void main() {
    final = vec4( 1.0 );
}