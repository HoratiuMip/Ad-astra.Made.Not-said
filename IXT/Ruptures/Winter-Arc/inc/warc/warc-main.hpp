#include <warc/common.hpp>
#include <warc/inet-tls.hpp>
#include <warc/satellite.hpp>

#include <IXT/descriptor.hpp>
#include <IXT/file-manip.hpp>

#include <boost/json.hpp>

namespace warc {


#define WARC_MAIN_STR WARC_STR"::Main"


class MAIN : IXT::Descriptor {
public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_MAIN_STR );

_WARC_PROTECTED:
    struct _N2YO {
        inet_tls::HBRIDGE    socket            = {};
        const char* const    request           = 
        /* Example: nid=25338 (NOAA-15) | lat=46.7, lng=23.56, alt=0 (Cluj-Napoca) | stp=1 */
        "GET /rest/v1/satellite/positions/%nid%/%lat%/%lng%/%alt%/%stp%/&apiKey=%apk% HTTP/1.1\r\n"\
        "Host: api.n2yo.com\r\n"\
        "\r\n";
        const char*          api_key           = nullptr;
        static const char*   API_KEY_ASH;
        static const int     API_KEY_ASH_LEN;
        static const int     API_KEY_SIG_LEN;

        std::string tufilin_request( 
            sat::NORAD_ID   norad_id, 
            WARC_FTYPE      obs_lat, 
            WARC_FTYPE      obs_lng, 
            WARC_FTYPE      obs_alt,
            int             steps
        ); 

        int burn_api_key( const char* key, const char* process );

    }   _n2yo   = {};

_WARC_PROTECTED:
    int _parse_opts( int argc, char* argv[] );

    int _parsing_proc_n2yo_api_key( char* argv[], const char* process );

public:
    int main( int argc, char* argv[], VOID_DOUBLE_LINK vdl );
};


};
