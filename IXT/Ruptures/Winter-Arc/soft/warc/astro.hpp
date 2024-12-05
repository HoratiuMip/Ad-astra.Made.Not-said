#pragma once

#include <warc/common.hpp>

namespace warc { namespace astro {


#define WARC_ASTRO_STR WARC_STR"::astro"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_ASTRO_STR );


std::pair< float, float > sun_lat_long_from_timestamp( time_t ts, time_t rve );

std::pair< float, float > sun_lat_long_now( time_t rve );


} };