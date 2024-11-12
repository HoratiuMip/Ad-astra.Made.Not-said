#version 410 core

in vec2 f_txt_crd;
in vec3 f_vrtx_n;
in vec3 f_sun_ray;

out vec4 chroma;

uniform sampler2D diffuse_texture;

void main() {
    chroma = vec4( vec3( dot( f_sun_ray, f_vrtx_n ) / ( length( f_sun_ray ) * length( f_vrtx_n ) ) * 2.0 ), 1.0 );
    chroma = max( chroma, vec4( .1 ) );
    chroma *= texture( diffuse_texture, f_txt_crd );
}