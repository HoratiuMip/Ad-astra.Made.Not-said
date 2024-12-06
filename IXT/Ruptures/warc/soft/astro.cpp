#include <warc/astro.hpp>

namespace warc { namespace astro {


std::pair< float, float > sun_lat_long_from_timestamp( time_t ts ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_ASTRO_STR"::sun_lat_long_from_timestamp()" );

    std::pair< float, float > res;

    res.first = params.EAID * sin( ( double )( ( ts - params.ref_vernal_equinox_ts ) % params.SIY ) / params.SIY * PI*2 );

    double year = ( double )ts / params.SIY - 30.0;
    double days = ( double )( ( ts - params.ref_first_january_ts ) % params.SIY ) / params.SID;
    double hour = ( double )( ts % params.SID ) / params.SID * 24.0;
    
    WARC_LOG_RT_INTEL << "Current year: " << year << " | Days since the first of January: " << days << " | Current hour: " << hour << ".";

    double D = 6.240'040'77 + 0.017'201'97*( 365.25*year + days );
    double d = -7.659*sin( D ) + 9.863*sin( 2*D + 3.5932 );

    res.second = -15.0*( hour - 12.0 + d/60.0 );

    WARC_LOG_RT_INTEL << "Approximated subsolar point @" << ts << ": lat " << res.first << " | long " << res.second << ".";

    return res;
}

std::pair< float, float > sun_lat_long_now() {
    return sun_lat_long_from_timestamp( time( nullptr ) );
}


} };