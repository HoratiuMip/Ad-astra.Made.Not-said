#version 410 core
//IXT#include <../common.glsl>
//IXT#include <../perlin.glsl>

in GS_OUT {
    vec2    tex_crd;
    vec3    nrm;
    vec3    sun_ray;
    vec3    sat2vrtx[ SAT_COUNT ];
    float   w_perl;
    vec3    lens2vrtx;
} gs_in;

out vec4 final;

uniform float     rtc;
uniform float     sat_high;
uniform vec3      sat_high_specs[ SAT_COUNT ];
uniform sampler2D map_Ka;
uniform sampler2D map_Kd;
uniform sampler2D map_Ks;

void main() {
    float light = dot( -gs_in.sun_ray, gs_in.nrm ) / ( length( gs_in.sun_ray ) * length( gs_in.nrm ) ) + 0.12;
    float dark = -light + 0.12;
    
    light = 1.6 * max( pow( light, 0.82 ), 0.06 ); 

    const float CITY_LIGHTS_BUMP = 0.56;
    vec4 city_lights = texture( map_Ka, gs_in.tex_crd );

    city_lights.r = pow( city_lights.r, CITY_LIGHTS_BUMP );
    city_lights.g = pow( city_lights.g, CITY_LIGHTS_BUMP );
    city_lights.b = pow( city_lights.b, CITY_LIGHTS_BUMP );
    city_lights *= 3.0;

    float land = texture( map_Ks, gs_in.tex_crd ).s;

    final = 
        texture( map_Kd, gs_in.tex_crd ) * light 
        + 
        max( city_lights * dark, vec4( 0.0 ) ) * vec4( 1.0, 1.0, 0.8, 1.0 )
        + 
        vec4( 0.0, 0.32, 0.62, 1.0 ) * ( 1.0 - land ) * gs_in.w_perl * light;

    if( 
        sat_high != 0.0 
        && 
        bool( int( floor( ( gs_in.tex_crd.x + gs_in.tex_crd.y ) * 100.0 ) ) & 1 ) 
    ) {
        // 3,116,988m --- sat tx radius
        // 6,378,137m --- earth radius
        // 809,000m   --- sat mean apogee perigee  // CAUTION: THIS HEAVILY ENJINIRD NUMBERS GAVE A HEAVILY ENJINIRD RESULT ( is pretty good actually nice )
        // 7,187,137m --- earth center to sat
        // 3,220,263m --- longest sat to earth
        const float OUTTER_DIST = 0.504;

        float longest_dist     = 0.0;
        int   furthest_sat_idx = -1;

        for( int sidx = 0; sidx < SAT_COUNT; ++sidx ) {
            float sat_dist = length( gs_in.sat2vrtx[ sidx ] );
            if( sat_dist > OUTTER_DIST ) continue;

            if( sat_dist > longest_dist ) {
                longest_dist = sat_dist;
                furthest_sat_idx = sidx;
            }
        }

        if( furthest_sat_idx >= 0 ) {
            vec3 sat_high_spec = sat_high_specs[ furthest_sat_idx ];

            if( sat_high_spec.gb != vec2( 0.0 ) ) {
                final.rgb = mix( final.rgb, sat_high_spec, sat_high_spec.r * pow( longest_dist / OUTTER_DIST, 4.0 ) ); 
            }
        }
    } 

    float lens_flare_deg = degs_btw( gs_in.nrm, gs_in.lens2vrtx );

    const float LENS_FBR = 16.0;
    const float LENS_FFR = 32.0;
    if( lens_flare_deg <= 90.0 + LENS_FBR && lens_flare_deg >= 90.0 - LENS_FFR ) {
        float diff   = 90.0 - lens_flare_deg;
        bool  is_fwd = diff >= 0.0; 

        final.rgb = mix( 
            final.rgb, vec3( ( 1.0 - light ) * 0.8, 0.62 + ( 1.0 - light ) * 0.18, 0.8 ), 
            ( 1.0 - abs( 90.0 - lens_flare_deg ) / float( is_fwd ? LENS_FFR : LENS_FBR ) ) * 0.6
        );
    }
    
    final.w = 1.0;
}