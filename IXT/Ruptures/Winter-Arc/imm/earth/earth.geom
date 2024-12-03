#version 410 core
//IXT#include <../common.glsl>
//IXT#include <../perlin.glsl>

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

in VS_OUT {
    vec2      tex_crd;
    vec3      nrm;
    vec3      sun_ray;
    flat vec3 lens;
    flat mat4 pv;
} vs_in[];

out GS_OUT {
    vec2      tex_crd;
    vec3      nrm;
    vec3      sun_ray;
    float     sat_dists[ SAT_COUNT ];
    float     w_perl;
    flat vec3 lens;
} gs_out;

uniform float     rtc;
uniform vec3      sat_poss[ SAT_COUNT ];
uniform sampler2D map_Ks;
uniform sampler2D map_Ns;

void main() { 
    vec3 trig_nrm = normalize( cross(
        vec3( gl_in[ 1 ].gl_Position - gl_in[ 0 ].gl_Position ),
        vec3( gl_in[ 2 ].gl_Position - gl_in[ 0 ].gl_Position )
    ) );

    vec2 perlin_fac = 22.2 * vec2( sin( rtc / 12.6 ), cos( rtc / 5.6 ) ) + vec2( 6.2 );

    for( int idx = 0; idx < 3; ++idx ) {
        float w_perl = max( perlin( abs( vec2( 0.5 ) - vs_in[ idx ].tex_crd ) * perlin_fac ), 0.0 );

        gs_out.tex_crd = vs_in[ idx ].tex_crd;
        gs_out.nrm     = vs_in[ idx ].nrm;
        gs_out.sun_ray = vs_in[ idx ].sun_ray;
        gs_out.w_perl  = w_perl;
        gs_out.lens    = vs_in[ idx ].lens;

        vec3  nrm = normalize( vs_in[ idx ].nrm ) * 0.08;
        float alt = sqrt( texture( map_Ns, vs_in[ idx ].tex_crd ).r );

        alt += ( 1.0 - texture( map_Ks, vs_in[ idx ].tex_crd ).r ) * w_perl * 0.18;

        gl_Position = ( gl_in[ idx ].gl_Position + vec4( nrm * alt, 0.0) );

        for( int sidx = 0; sidx < SAT_COUNT; ++sidx ) {
            gs_out.sat_dists[ sidx ] = distance( vec3( gl_Position ), sat_poss[ sidx ] );
        }

        gl_Position = vs_in[ 0 ].pv * gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
