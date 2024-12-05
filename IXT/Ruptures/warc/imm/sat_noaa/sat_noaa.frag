#version 410 core

in VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
    vec3 sun_ray;
    vec3 lens_ray;
} vs_out;

out vec4 final;

uniform float     sat_high;      
uniform vec3      high_spec;
uniform sampler2D map_Kd;

void main() {
    float depth;

    final = texture( map_Kd, vs_out.tex_crd );
    final.r *= 0.86;

    final *= ( 1.0 - sat_high );
    final += vec4( high_spec, 0.0 ) * sat_high;
    
    gl_FragDepth = gl_FragCoord.z * ( 1.0 - sat_high );
    final.w = 1.0;
}