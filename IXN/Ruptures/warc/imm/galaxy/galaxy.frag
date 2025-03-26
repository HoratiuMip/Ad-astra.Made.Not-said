#version 410 core
//IXN#name<galaxy-frag>
//IXN#include<../perlin.glsl>

in VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
} vs_out;

out vec4 final;

uniform sampler2D map_Kd;
uniform sampler2D IXN_map_shine;

uniform float rtc;

void main() {
    vec4 tex_frag = texture( map_Kd, vs_out.tex_crd );
    vec3 grn_fac  = texture( IXN_map_shine, vs_out.tex_crd ).rgb;

    bool is_star = grn_fac.g > 0.5 && grn_fac.r < 0.5 && grn_fac.b < 0.5;

    float perl_fac = perlin( vs_out.tex_crd * ( 56.2 + sin( rtc / 6.2 ) * 6.2 ) + vec2( sin( rtc / 4.2 ), cos( rtc / 4.2 ) ) * 6.2 );

    final = tex_frag * float( !is_star );
    final += tex_frag * vec4( max( vec3( 3.6 * perl_fac ), vec3( 0.48 ) ), 1.0 ) * float( is_star );
}