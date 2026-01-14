struct POSITION {
    double    satlatitude;
    double    satlongitude;
    double    sataltitude;
    double    azimuth;
    double    elevation;
    double    ra;
    double    dec;
    int64_t   timestamp;
};

#pragma once

#include <warc/common.hpp>

namespace warc { namespace sat {


#define WARC_SAT_STR WARC_STR"::sat"
_WARC_IXN_COMPONENT_DESCRIPTOR( WARC_SAT_STR );


struct POSITION {
    double    satlatitude;
    double    satlongitude;
    double    sataltitude;
    double    azimuth;
    double    elevation;
    double    ra;
    double    dec;
    int64_t   timestamp;
    bool      eclipsed;
};


enum NORAD_ID : int64_t {
    NORAD_ID_NOAA_15 = 25338,
    NORAD_ID_NOAA_18 = 28654,
    NORAD_ID_NOAA_19 = 33591,

    _NORAD_ID_FORCE_QWORD = 0x7F'FF'FF'FF'FF'FF'FF'FF
};


enum STATUS : int32_t {
    STATUS_UNKNOWN  = 0,
    STATUS_IN_ORBIT = 1,
    STATUS_DECAYED  = 2,

    _STATUS_FORCE_DWORD = 0x7F'FF'FF'FF
};


} };