#version 410 core

in vec2 f_txt_crd;
in vec3 f_vrtx_n;
in vec3 f_sun_ray;

out vec4 final_chroma;

uniform sampler2D diffuse_texture;
uniform sampler2D ambient_texture;

void main() {
    float light_factor = dot( f_sun_ray, f_vrtx_n ) / ( length( f_sun_ray ) * length( f_vrtx_n ) );
    float dark_factor = ( -light_factor + .16 ) / 1.1;

    light_factor *= 2.0;
    light_factor = max( light_factor, .072 );

    vec4 city_light = texture( ambient_texture, f_txt_crd ) * 3.0;

    final_chroma = texture( diffuse_texture, f_txt_crd )*light_factor + max( city_light*dark_factor, vec4( 0 ) );
}