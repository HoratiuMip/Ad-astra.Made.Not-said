#include <warc/warc-main.hpp>

namespace warc {


std::string MAIN::_N2YO::tufilin_request( 
    sat::NORAD_ID   norad_id, 
    WARC_FTYPE      obs_lat, 
    WARC_FTYPE      obs_lng, 
    WARC_FTYPE      obs_alt,
    int             steps
) {
    WARC_ASSERT_RT( steps > 0, "Cannot obtain positions for steps <= 0.", steps, "" );
    WARC_ASSERT_RT( this->api_key != nullptr, "API key not loaded. Use the --n2yo-api-key option.", -1, "" );
    
    std::string result = this->request;

    const char* procs[] = {
        "%nid%", "%lat%", "%lng%", "%alt%","%stp%", "%apk%"
    };

    const char* REDACTED_APK = "%WARC_REDACTED_APK%";

    auto cvt_arg = [ &, this ] ( int idx ) -> std::string {
        switch( idx ) {
            case 0: return std::to_string( norad_id );
            case 1: return std::to_string( obs_lat );
            case 2: return std::to_string( obs_lng );
            case 3: return std::to_string( obs_alt );
            case 4: return std::to_string( steps );
            case 5: return REDACTED_APK;
        }

        WARC_ASSERT_RT( false, "Invalid argument index for conversion.", idx, "0" );
        return ""; /* UNREACHABLE */
    };

    size_t pos = std::string::npos;

    for( int idx = 0; idx < sizeof( procs ) / sizeof( char* ); ++idx ) {
        pos = result.find( procs[ idx ] );

        WARC_ASSERT_RT( pos != std::string::npos, "Missing replacement string.", procs[ idx ], "" );

        result = result.replace( pos, strlen( procs[ idx ] ), cvt_arg( idx ) );
    }

    WARC_LOG_RT_OK << "Tufillin\'d n2yo request:\n" << result;

    return result.replace( pos, strlen( REDACTED_APK ), this->api_key );
}


int MAIN::main( int argc, char* argv[], VOID_DOUBLE_LINK vdl ) {
    int status = -1;

    status = inet_tls::uplink( {} );

    WARC_ASSERT_RT( status == 0, "Fault at starting the <inet_tls> module.", status, status );

    auto request = _n2yo.tufilin_request( sat::NORAD_ID_NOAA_15, 0, 0, 0, 5 );

    _n2yo.socket = inet_tls::BRIDGE::alloc( "158.69.117.9", inet_tls::INET_PORT_HTTPS );
  
    auto response = _n2yo.socket->xchg( request.c_str(), request.size(), 1000 );

    inet_tls::BRIDGE::free( std::move( _n2yo.socket ) );

    std::cout << response << '\n';

    auto idx = response.find( "\r\n\r\n" ) + 2;
    auto sz = atoi( response.c_str() + idx );

    std::string json{ response.c_str() + idx };

    std::cout << json;


    status = inet_tls::downlink( {} );
    return status;
}


};
