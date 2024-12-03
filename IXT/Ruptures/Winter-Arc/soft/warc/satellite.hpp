#pragma once

#include <warc/common.hpp>

namespace warc { namespace sat {


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


} };