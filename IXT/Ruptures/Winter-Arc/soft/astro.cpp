#include <warc/astro.hpp>

namespace warc { namespace astro {


std::pair< float, float > sun_lat_long_from_timestamp( time_t ts, time_t rve ) {
    static constexpr time_t sbe = 31'556'520;
    static constexpr time_t srr = 86'400;

    std::pair< float, float > res;

    res.first = 23.5 * cos( ( ts - rve ) % sbe );
    res.second = 360.0 * ( ( ts - rve ) % srr ) / srr;

    return res;
}

std::pair< float, float > sun_lat_long_now( time_t rve ) {
    return sun_lat_long_from_timestamp( time( nullptr ), rve );
}


} };