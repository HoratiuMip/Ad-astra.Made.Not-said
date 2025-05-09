#version 410 core
//IXN#name<barracuda-ctrl-frag>

in VS_OUT {
    vec2 tex_crd;
    vec3 nrm;
    vec3 lens2vrtx;
} vs_out;

out vec4 final;

uniform vec3      Kd;
uniform sampler2D map_Kd;

float cos_btw( in vec3 v1, in vec3 v2 ) {
    return dot( v1, v2 ) / ( length( v1 ) * length( v2 ) );
} 

void main() {
    final = Kd.s > 0.0 ? vec4( Kd, 1.0 ) : texture( map_Kd, vs_out.tex_crd );
    vec3 mod = vec3( max( 0.0, pow( cos_btw( vs_out.nrm, -vs_out.lens2vrtx ), 5.0 ) ) ) * 0.26;

    if( final.x + mod.x < 0.5 ) final.x += mod.x;
    if( final.y + mod.y < 0.5 ) final.y += mod.y;
    if( final.z + mod.z < 0.5 ) final.z += mod.z;
}