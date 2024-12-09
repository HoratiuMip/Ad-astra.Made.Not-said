const int     SAT_COUNT   = 3;
const float   PI          = 3.14159265359;

float degs_btw( vec3 v1, vec3 v2 ) {
    return degrees( acos( dot( v1, v2 ) / ( length( v1 ) * length( v2 ) ) ) );
}