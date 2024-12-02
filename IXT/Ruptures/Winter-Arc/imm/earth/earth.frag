#version 410 core
//IXT#include <../perlin.glsl>

in GS_OUT {
    vec2      tex_crd;
    vec3      nrm;
    vec3      sun_ray;
    float     sat_dists[ 3 ];
    float     w_perl;
    flat vec3 lens;
} gs_in;

out vec4 final;

uniform float     rtc;
uniform vec4      highlight;
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

    if( highlight.a != 0.0 && bool( int( floor( ( gs_in.tex_crd.x + gs_in.tex_crd.y ) * 100.0 ) ) & 1 ) ) {
        // 3,116,988m --- sat tx radius
        // 6,378,137m --- earth radius
        // 809,000m   --- sat mean apogee perigee  // CAUTION: THIS HEAVILY ENJINIRD NUMBERS GAVE A HEAVILY ENJINIRD RESULT ( is pretty good actually nice )
        // 7,187,137m --- earth center to sat
        // 3,220,263m --- longest sat to earth
        const float OUTTER_DIST = 0.504;

        float longest_dist = 0.0;

        for( int sidx = 0; sidx < 3; ++sidx ) {
            if( gs_in.sat_dists[ sidx ] > OUTTER_DIST ) continue;

            if( gs_in.sat_dists[ sidx ] > longest_dist ) longest_dist = gs_in.sat_dists[ sidx ];
        }

        final.rgb = mix( final.rgb, vec3( 1.0, 0.0, 0.82 ), highlight.r * pow( longest_dist / OUTTER_DIST, 4.0 ) ); 
    } 

    final.w = 1.0;
}