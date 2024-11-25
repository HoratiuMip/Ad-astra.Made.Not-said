#pragma once

#include <warc/common.hpp>
#include <warc/inet-tls.hpp>
#include <warc/satellite.hpp>

#include <IXT/descriptor.hpp>
#include <IXT/file-manip.hpp>

#include <boost/json.hpp>

#include <vector>

namespace warc { namespace n2yo {


#define WARC_N2YO_STR WARC_STR"::n2yo"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_N2YO_STR );


struct POSITIONS {
    static inline const char* const JSON_SAMPLE = "{\"info\":{\"satname\":\"NOAA 15\",\"satid\":25338,\"transactionscount\":0},\"positions\":[{\"satlatitude\":80.16280485,\"satlongitude\":32.88210375,\"sataltitude\":821.95,\"azimuth\":5.41,\"elevation\":-36.94,\"ra\":63.20914143,\"dec\":52.64921221,\"timestamp\":1731589629,\"eclipsed\":false},{\"satlatitude\":80.19217995,\"satlongitude\":32.57743619,\"sataltitude\":821.95,\"azimuth\":5.35,\"elevation\":-36.94,\"ra\":63.29149817,\"dec\":52.6591026,\"timestamp\":1731589630,\"eclipsed\":false},{\"satlatitude\":80.2212889,\"satlongitude\":32.27098941,\"sataltitude\":821.95,\"azimuth\":5.29,\"elevation\":-36.94,\"ra\":63.37390758,\"dec\":52.66890195,\"timestamp\":1731589631,\"eclipsed\":false},{\"satlatitude\":80.25013047,\"satlongitude\":31.96275143,\"sataltitude\":821.95,\"azimuth\":5.23,\"elevation\":-36.94,\"ra\":63.45637255,\"dec\":52.67861055,\"timestamp\":1731589632,\"eclipsed\":false},{\"satlatitude\":80.27869995,\"satlongitude\":31.65274793,\"sataltitude\":821.95,\"azimuth\":5.17,\"elevation\":-36.93,\"ra\":63.53888602,\"dec\":52.68822755,\"timestamp\":1731589633,\"eclipsed\":false}]}";

    struct INFO {
        std::string     satname;
        sat::NORAD_ID   satid; 
        int64_t         transactionscount;      
    };

    INFO                           info;
    std::vector< sat::POSITION >   data;

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