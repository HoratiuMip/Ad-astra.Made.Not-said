#version 410 core


// Perlin noise generator taken from here: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
vec4 perlin_permute( vec4 x ) { return mod( ( ( x * 34.0 ) + 1.0 ) * x, 289.0 ); }
vec2 perlin_fade( vec2 t ) { return t * t * t * ( t * ( t * 6.0 - 15.0 ) + 10.0 ); }
float perlin( vec2 P ){
  vec4 Pi = floor( P.xyxy ) + vec4( 0.0, 0.0, 1.0, 1.0 );
  vec4 Pf = fract( P.xyxy ) - vec4( 0.0, 0.0, 1.0, 1.0 );
  Pi = mod( Pi, 289.0 );
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = perlin_permute( perlin_permute( ix ) + iy );
  vec4 gx = 2.0 * fract( i * 0.0243902439 ) - 1.0;
  vec4 gy = abs( gx ) - 0.5;
  vec4 tx = floor( gx + 0.5 );
  gx = gx - tx;
  vec2 g00 = vec2( gx.x,gy.x );
  vec2 g10 = vec2( gx.y,gy.y );
  vec2 g01 = vec2( gx.z,gy.z );
  vec2 g11 = vec2( gx.w,gy.w );
  vec4 norm = 1.79284291400159 - 0.85373472095314 * vec4( dot( g00, g00 ), dot( g01, g01 ), dot( g10, g10 ), dot( g11, g11 ) );
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot( g00, vec2( fx.x, fy.x ) );
  float n10 = dot( g10, vec2( fx.y, fy.y ) );
  float n01 = dot( g01, vec2( fx.z, fy.z ) );
  float n11 = dot( g11, vec2( fx.w, fy.w ) );
  vec2 fade_xy = perlin_fade( Pf.xy );
  vec2 n_x = mix( vec2( n00, n01 ), vec2( n10, n11 ), fade_xy.x );
  float n_xy = mix( n_x.x, n_x.y, fade_xy.y );

  return 2.3 * n_xy;
}


in vec2 f_txt;
in vec3 f_nrm;
in vec3 f_sun_ray;

out vec4 final;

uniform float     perlin_fac;
uniform sampler2D ambient_tex;
uniform sampler2D diffuse_tex;
uniform sampler2D specular_tex;

void main() {
    float light = dot( f_sun_ray, f_nrm ) / ( length( f_sun_ray ) * length( f_nrm ) );
    float dark = ( -light + .16 ) / 1.16;

    light = 2.0 * max( light, .06 );

    vec4 city_lights = texture( ambient_tex, f_txt ) * 3.0;

    final = 
        texture( diffuse_tex, f_txt ) * light 
        + 
        max( city_lights * dark, vec4( 0.0 ) ) * vec4( 1.0, 1.0, 0.8, 1.0 ) 
        + 
        vec4( 0.0, 0.4, 0.7, 1.0 ) * ( 1.0 - texture( specular_tex, f_txt ).x ) * max( perlin( abs( vec2( 0.5 ) - f_txt ) * perlin_fac ), 0.0 ) * light;

    final.w = 1.0;
}