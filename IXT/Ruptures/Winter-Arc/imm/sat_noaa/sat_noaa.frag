#version 410 core

in vec2 f_txt;
in vec3 f_nrm;
in vec3 f_sun_ray;

out vec4 final;

uniform sampler2D diffuse_tex;

void main() {
    float light = dot( f_sun_ray, f_nrm ) / ( length( f_sun_ray ) * length( f_nrm ) );

    light *= 1.6;
    light = max( light, .0 );

    final = texture( diffuse_tex, f_txt ) * ( .5 + light );
    final.r *= 0.82;
    final.w = 1.0;
}