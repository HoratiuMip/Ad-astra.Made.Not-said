#version 410 core

layout( triangles ) in;
layout( triangle_strip, max_vertices = 6 ) out;

in VS_OUT {
    vec2      tex_crd;
    vec3      nrm;
    vec3      sun_ray;
    flat vec3 lens;
    flat mat4 proj;
} vs_in[];

out GS_OUT {
    vec2      tex_crd;
    vec3      nrm;
    vec3      sun_ray;
    flat vec3 lens;
} gs_out;

const float ATM_HEIGHT = 0.01;

void main() {
    vec4 trig_nrm = vec4( normalize( cross(
        ( gl_in[ 1 ].gl_Position - gl_in[ 0 ].gl_Position ).xyz,
        ( gl_in[ 2 ].gl_Position - gl_in[ 0 ].gl_Position ).xyz
    ) ), 0.0 );

    for( int vidx = 0; vidx < 3; ++vidx ) {
        gl_Position    = vs_in[ vidx ].proj * gl_in[ vidx ].gl_Position;
        gs_out.tex_crd = vs_in[ vidx ].tex_crd;
        gs_out.nrm     = vs_in[ vidx ].nrm;
        gs_out.sun_ray = vs_in[ vidx ].sun_ray;
        gs_out.lens    = vs_in[ vidx ].lens;
        EmitVertex();
    }
    EndPrimitive();
}
