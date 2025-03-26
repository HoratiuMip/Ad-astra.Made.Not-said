#pragma once

#include <warc/common.hpp>

#include <glm/glm.hpp>

namespace warc { namespace astro {


#define WARC_ASTRO_STR WARC_STR"::astro"
_WARC_IXN_COMPONENT_DESCRIPTOR( WARC_ASTRO_STR );


struct LAT_LONG {
    float   lat   = 0.0;
    float   lng   = 0.0;
};


struct PARAMS {
    const double   EAID  = 23.5;       /* Earth axis inclination degrees. */
    const time_t   SIY   = 31'556'926; /* Seconds in year. */
    const time_t   SID   = 86'400;     /* Seconds in day.*/

    time_t   ref_vernal_equinox_ts   = 1'679'347'500; /* 2023's Vernal Equinox. */
    time_t   ref_first_january_ts    = 1'704'067'200; /* 1 JAN 2024. */

}; inline PARAMS params;


LAT_LONG sun_lat_long_from_timestamp( time_t ts );
LAT_LONG sun_lat_long_now();

glm::vec4 nrm_from_lat_long( LAT_LONG ll );


} };