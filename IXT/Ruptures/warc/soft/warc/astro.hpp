#pragma once

#include <warc/common.hpp>

namespace warc { namespace astro {


#define WARC_ASTRO_STR WARC_STR"::astro"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_ASTRO_STR );


struct PARAMS {
    const double   EAID  = 23.5;
    const time_t   SIY   = 31'556'926;
    const time_t   SID   = 86'400;

    time_t   ref_vernal_equinox_ts   = -1;
    time_t   ref_first_january_ts    = -1;

}; inline PARAMS params;


std::pair< float, float > sun_lat_long_from_timestamp( time_t ts );

std::pair< float, float > sun_lat_long_now();


} };