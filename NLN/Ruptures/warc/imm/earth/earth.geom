#version 410 core
//NLN#name<earth-geom>
//NLN#include<../common.glsl>
//NLN#include<../perlin.glsl>

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

in VS_OUT {
    vec2   tex_crd;
    vec3   nrm;
    vec3   sun_ray;
} vs_in[];

out GS_OUT {
    vec2    tex_crd;
    vec3    nrm;
    vec3    sun_ray;
    vec3    sat2vrtx[ SAT_COUNT ];
    float   w_perl;
    vec3    lens2vrtx;
} gs_out;

uniform float     rtc;
uniform vec3      sat_poss[ SAT_COUNT ];
uniform vec3      lens_pos;
uniform sampler2D NLN_map_cal;
uniform mat4      proj;
uniform mat4      view;

void main() { 
    vec2 perlin_fac = 22.2 * vec2( sin( rtc / 12.6 ), cos( rtc / 5.6 ) ) + vec2( 6.2 );
    
    for( int idx = 0; idx < 3; ++idx ) {
        float w_perl = max( perlin( abs( vec2( 0.5 ) - vs_in[ idx ].tex_crd ) * perlin_fac ), 0.0 );
        vec4  cal    = texture( NLN_map_cal, vs_in[ idx ].tex_crd );

        gs_out.tex_crd = vs_in[ idx ].tex_crd;
        gs_out.nrm     = vs_in[ idx ].nrm;
        gs_out.sun_ray = vs_in[ idx ].sun_ray;
        gs_out.w_perl  = w_perl;

        vec3  nrm = normalize( vs_in[ idx ].nrm ) * 0.08;
        float alt = sqrt( cal.g );

        alt += ( 1.0 - float( cal.b > 0.1 ) ) * w_perl * 0.16;

        gl_Position = ( gl_in[ idx ].gl_Position + vec4( nrm * alt, 0.0) );

        gs_out.lens2vrtx = vec3( gl_Position ) - lens_pos;

        for( int sidx = 0; sidx < SAT_COUNT; ++sidx ) {
            gs_out.sat2vrtx[ sidx ] = vec3( gl_Position ) - sat_poss[ sidx ];
        }

        gl_Position = proj * view * gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
