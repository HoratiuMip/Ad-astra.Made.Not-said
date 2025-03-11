#version 410 core
//NLN#name<barracuda-ctrl-frag>

in VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
    vec3 lens2vrtx;
} vs_out;

out vec4 final;

uniform sampler2D map_Kd;

void main() {
    final = texture( map_Kd, vs_out.tex_crd );
}