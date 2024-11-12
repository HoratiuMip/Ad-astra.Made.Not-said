#pragma once

#include <warc/common.hpp>
#include <warc/inet-tls.hpp>
#include <warc/satellite.hpp>

#include <IXT/descriptor.hpp>
#include <IXT/file-manip.hpp>

#include <boost/json.hpp>

namespace warc { namespace n2yo {


#define WARC_N2YO_STR WARC_STR"::n2yo"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_N2YO_STR );


struct POSITIONS {

};


struct _N2YO {
    inet_tls::HBRIDGE    socket    = {};
    const char* const    request   = 
    /* Example: nid=25338 (NOAA-15) | lat=46.7, lng=23.56, alt=0 (Cluj-Napoca) | stp=1 */
    "GET /rest/v1/satellite/positions/%nid%/%lat%/%lng%/%alt%/%stp%/&apiKey=%apk% HTTP/1.1\r\n"\
    "Host: api.n2yo.com\r\n"\
    "\r\n";
    std::string          api_key   = {};

    static const char*   API_KEY_ASH;
    static const int     API_KEY_ASH_LEN;
    static const int     API_KEY_ASH_SIG_LEN;

    std::string tufilin_positions_request( 
        sat::NORAD_ID   norad_id, 
        WARC_FTYPE      obs_lat, 
        WARC_FTYPE      obs_lng, 
        WARC_FTYPE      obs_alt,
        int             steps
    );

    std::string send_get_positions(
        sat::NORAD_ID   norad_id, 
        WARC_FTYPE      obs_lat, 
        WARC_FTYPE      obs_lng, 
        WARC_FTYPE      obs_alt,
        int             steps
    );

    POSITIONS json_2_positions( std::string_view json );

    int burn_api_key( const char* key, const char* process );
    std::string extract_api_key( const char* process );

};


} };