#version 410 core
//IXT#include <../perlin.glsl>

in GS_OUT {
    vec2      tex_crd;
    vec3      nrm;
    vec3      sun_ray;
    float     w_perl;
    flat vec3 lens;
} gs_in;

out vec4 final;

uniform float     rtc;
uniform sampler2D map_Ka;
uniform sampler2D map_Kd;
uniform sampler2D map_Ks;

void main() {
    float light = dot( gs_in.sun_ray, gs_in.nrm ) / ( length( gs_in.sun_ray ) * length( gs_in.nrm ) );
    float dark = ( -light + .16 ) / 1.16;

    light = 2.0 * max( light, .06 );

    vec4 city_lights = texture( map_Ka, gs_in.tex_crd ) * 3.0;

    final = 
        texture( map_Kd, gs_in.tex_crd ) * light 
        + 
        max( city_lights * dark, vec4( 0.0 ) ) * vec4( 1.0, 1.0, 0.8, 1.0 ) 
        + 
        vec4( 0.0, 0.4, 0.7, 1.0 ) * ( 1.0 - texture( map_Ks, gs_in.tex_crd ).x ) * gs_in.w_perl * light;

    final.w = 1.0;
}