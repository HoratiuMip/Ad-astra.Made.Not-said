#version 410 core

layout( location = 0 ) in vec3 vrtx;
layout( location = 1 ) in vec3 vrtx_n;
layout( location = 2 ) in vec2 txt_crd;

out vec2 f_txt_crd;
out vec3 f_vrtx_n;
out vec3 f_sun_ray;

uniform vec3 u_sun_pos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    f_txt_crd = txt_crd;

    f_sun_ray = u_sun_pos - vrtx;
    f_vrtx_n = vrtx_n;

    gl_Position = u_projection * u_view * u_model * vec4( vrtx, 1.0 );
}
