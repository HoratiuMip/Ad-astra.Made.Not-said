#version 410 core

in vec2 f_txt;
in vec3 f_nrm;
in vec3 f_sun_ray;

out vec4 final;

uniform sampler2D ambient_texture;
uniform sampler2D diffuse_texture;

void main() {
    float light = dot( f_sun_ray, f_nrm ) / ( length( f_sun_ray ) * length( f_nrm ) );
    float dark = ( -light + .16 ) / 1.16;

    light *= 2.0;

    vec4 city_lights = texture( ambient_texture, f_txt ) * 3.0;

    final = texture( diffuse_texture, f_txt )*light + max( city_lights*dark, vec4( 0 ) );
}