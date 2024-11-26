#version 410 core

in VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
} vs_out;

out vec4 final;

uniform sampler2D diffuse_tex;
uniform sampler2D specular_tex;

uniform float rtc;

void main() {
    vec4 tex_frag = texture( diffuse_tex, vs_out.tex_crd );
    vec3 grn_fac  = texture( specular_tex, vs_out.tex_crd ).rgb;

    final = tex_frag * vec4( max( 
        vec3( 3.0 * ( sin( 3.6 * rtc * ( vs_out.tex_crd.x + vs_out.tex_crd.y ) ) )  )
        * 
        float( grn_fac.g > 0.5 && grn_fac.r < 0.5 && grn_fac.b < 0.5 ), 
        vec3( 1.0 ) ),
        1.0 
    );
}