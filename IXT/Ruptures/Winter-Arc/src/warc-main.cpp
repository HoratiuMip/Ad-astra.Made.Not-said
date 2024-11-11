#include <warc/warc-main.hpp>

namespace warc {


const char*  MAIN::_N2YO::API_KEY_ASH          = "WARC-API-KEY-ASH-XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
const int    MAIN::_N2YO::API_KEY_ASH_LEN      = strlen( MAIN::_N2YO::API_KEY_ASH ); 
const int    MAIN::_N2YO::API_KEY_ASH_SIG_LEN  = 17; /* strlen( WARC-API-KEY-ASH- ) */

int MAIN::_N2YO::burn_api_key( const char* key, const char* process ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_N2YO::burn_api_key()" );

    WARC_ASSERT_RT( process != nullptr, "Process is NULL.", -1, -1 );
    WARC_ASSERT_RT( key != nullptr, "Key is NULL.", -1, -1 );
    
    WARC_LOG_RT_INTEL << "Burning key into the mirror of \"" << process << "\".";

    int key_len = strlen( key );
    WARC_ASSERT_RT( key_len <= API_KEY_ASH_LEN - API_KEY_ASH_SIG_LEN, "Key too long to burn.", key_len, -1 );

    std::ifstream file_read{ process, std::ios_base::binary };
    WARC_ASSERT_RT( file_read, "Could not open process file for read.", -1, -1 );

    auto sz = IXT::File::byte_count( file_read );
    WARC_ASSERT_RT( sz != 0, "The process' file where to burn the key has a size of 0 bytes.", -1, -1 );

    IXT::UPtr< char[] > buffer{ ( char* )malloc( sz ) };
    WARC_ASSERT_RT( buffer, "Could not allocate for the process file read.", -1, -1 );

    file_read.read( buffer.get(), sz );
    file_read.close();

    WARC_LOG_RT_INTEL << "Process file mirrored.";

    std::string_view  buf_view{ buffer.get(), sz };
    std::string       cmp{ API_KEY_ASH, API_KEY_ASH_SIG_LEN }; 

    auto pos = buf_view.find( cmp.c_str() );

    WARC_ASSERT_RT( pos != std::string_view::npos, "Could not detect any key ash signature.", -1, -1 );
    WARC_ASSERT_RT( 
        ( buf_view.find( cmp.c_str(), pos + API_KEY_ASH_SIG_LEN ) == std::string_view::npos ),
        "More than one key ash signatures detected. Cannot decide where to burn.",
        -1, -1
    );

    WARC_LOG_RT_INTEL << "Burning into process file mirror.";

    char* ash_begin = buffer.get() + pos + API_KEY_ASH_SIG_LEN;
    char* ash_end   = buffer.get() + pos + API_KEY_ASH_LEN;
    {
    int idx = 0;
    for( ; ( ash_begin + idx < ash_end ) && ( idx < strlen( key ) ); ++idx )
        ash_begin[ idx ] = key[ idx ];
    ash_begin[ idx ] = '\0';
    }
    
    WARC_LOG_RT_INTEL << "Writing process file mirror.";

    std::string mirror_process{ process };
    mirror_process += "(N2YO-API-KEY-BURNT)";

    std::ofstream file_write{ mirror_process.c_str(), std::ios_base::binary };

    if( !file_write ) {
        WARC_LOG_RT_ERROR << "Could not open file \"" << mirror_process << "\" for key burn.";
        return -1;
    }

    file_write.write( buffer.get(), sz );
    file_write.close();

    WARC_LOG_RT_OK << "Burnt key into \"" << mirror_process << "\".";

    return 0;
}

std::string MAIN::_N2YO::extract_api_key( const char* process ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_N2YO::extract_api_key()" );

    WARC_ASSERT_RT( process != nullptr, "Process is NULL.", -1, "" );

    WARC_LOG_RT_INTEL << "Extracting key from \"" << process << "\".";

    std::ifstream file_read{ process, std::ios_base::binary };
    WARC_ASSERT_RT( file_read, "Could not open process file.", -1, "" );

    auto sz = IXT::File::byte_count( file_read );
    WARC_ASSERT_RT( sz > API_KEY_ASH_LEN, "Cannot extract key from a file smaller than the key itself.", sz, "" );

    IXT::UPtr< char[] > buffer{ ( char* )malloc( sz ) };
    WARC_ASSERT_RT( buffer, "Could not allocate for the process file read.", -1, "" );

    file_read.read( buffer.get(), sz );
    file_read.close();

    WARC_LOG_RT_INTEL << "Process file read.";

    std::string_view  buf_view{ buffer.get(), sz };
    std::string       cmp{ API_KEY_ASH, API_KEY_ASH_SIG_LEN }; 

    auto pos = buf_view.find( cmp.c_str() );

    WARC_ASSERT_RT( pos != std::string_view::npos, "Could not detect any key ash signature.", -1, "" );
    WARC_ASSERT_RT( 
        ( buf_view.find( cmp.c_str(), pos + API_KEY_ASH_SIG_LEN ) == std::string_view::npos ),
        "More than one key ash signatures detected. Cannot decide from where to extract key.",
        -1, ""
    );

    std::string key; key.reserve( API_KEY_ASH_LEN - API_KEY_ASH_SIG_LEN );

    for( int idx = 0; ( buffer[ pos + idx ] != '\0' ) && ( idx < API_KEY_ASH_LEN - API_KEY_ASH_SIG_LEN ); ++idx )
        key += buffer[ pos + API_KEY_ASH_SIG_LEN + idx ];

    WARC_LOG_RT_OK << "Extracted key.";

    return key;
}

std::string MAIN::_N2YO::tufilin_request( 
    sat::NORAD_ID   norad_id, 
    WARC_FTYPE      obs_lat, 
    WARC_FTYPE      obs_lng, 
    WARC_FTYPE      obs_alt,
    int             steps
) {
    WARC_ASSERT_RT( steps > 0, "Cannot obtain positions for steps <= 0.", steps, "" );
    WARC_ASSERT_RT( !this->api_key.empty(), "API key not loaded. Use the --n2yo-api-key option.", -1, "" );
    
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


int MAIN::_parse_proc_n2yo_api_key( char* argv[], const char* process ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_n2yo_api_key()" );

    WARC_ASSERT_RT( argv != nullptr, "Argument array is NULL.", -1, -1 );
    WARC_ASSERT_RT( process != nullptr, "Process is NULL.", -1, -1 );
    WARC_ASSERT_RT( argv[ 0 ] != nullptr, "Api key is NULL.", -1, -1 );
    WARC_ASSERT_RT( argv[ 1 ] != nullptr, "Api key usage mode is NULL.", -1, -1 );

    struct _SWITCH {
        const char*   str;
        void*         lbl;
    } switches[] = {
        { str: "burn", lbl: &&l_burn_key },
        { str: "use",  lbl: &&l_use_mode },
        { str: "show", lbl: &&l_show_ash }
    };
    for( auto& sw : switches ) if( strcmp( argv[ 0 ], sw.str ) == 0 ) goto *sw.lbl;

    WARC_ASSERT_RT( false, "Invalid arguments.", -1, -1 );

l_burn_key: {
    return this->_n2yo.burn_api_key( argv[ 1 ], process );
}
l_use_mode: {
    if( strcmp( argv[ 1 ], "ash" ) == 0 ) {
        auto key = this->_n2yo.extract_api_key( process );
        WARC_ASSERT_RT( !key.empty(), "Could not extract key.", argv[ 1 ], -1 );

        this->_n2yo.api_key = std::move( key );
        WARC_LOG_RT_OK << "Key extracted and loaded.";

        return 0;
    }

    this->_n2yo.api_key = argv[ 1 ];
    WARC_LOG_RT_OK << "Key loaded.";

    return 0;
}
l_show_ash: {
    WARC_ASSERT_RT( strcmp( argv[ 1 ], "ash" ) == 0, "Invalid arguments.", argv[ 1 ], -1 );

    auto key = this->_n2yo.extract_api_key( process );
    
    if( key.empty() )
        WARC_LOG_RT_OK << "This process, \"" << process << "\", has no burnt key.";
    else
        WARC_LOG_RT_OK << "This process, \"" << process << "\", has the key \"" << key << "\" burnt.";

    return 0;
}

}

int MAIN::_parse_opts( int argc, char* argv[] ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_opts()" );

    WARC_LOG_RT_INTEL << "Starting option parsing."; 

    struct _OPT {
        const char*   str;
        int           argc;
        int           ( MAIN::*proc )( char**, const char* );
    } opts[] = {
        { "--n2yo-api-key", 2, &_parse_proc_n2yo_api_key }
    };
    const int optc = sizeof( opts ) / sizeof( _OPT );

    for( int idx = 1; idx < argc; ++idx ) {
        char*   opt      = argv[ idx ];
        char**  opt_ptr  = argv + idx;

        if( !std::string_view{ opt }.starts_with( "--" ) ) continue;

        _OPT* record = std::find_if( opts, opts + optc, [ &opt ] ( _OPT& rec ) -> bool {
            return strcmp( opt, rec.str ) == 0;
        } );

        if( record == opts + optc ) {
            WARC_LOG_RT_WARNING << "Ignoring invalid option \"" << opt << "\".";
            continue;
        }

        if( 
            ( idx + 1 + record->argc > argc ) 
            ||
            ( [ &idx, &argv, &argc, &record ] () mutable -> bool { 
                int step = 1;
                for( ; step <= record->argc && idx + 1 < argc; ++step, ++idx )
                    if( std::string_view{ argv[ idx + 1 ] }.starts_with( "--" ) )
                        break;

                return step == record->argc;
            } )()
        ) {
            WARC_LOG_RT_ERROR << "Not enough arguments for option \"" << opt << "\", (" << record->argc << ") required.\n";
            return -1;
        }

        WARC_LOG_RT_INTEL << "Executing procedure for option \"" << opt << "\".";

        int status = ( this->*( record->proc ) )( opt_ptr + 1, argv[ 0 ] );
        WARC_ASSERT_RT( status == 0, "Option procedure exited abnormally.\n", status, status );

        WARC_LOG_RT_OK << "Procedure for option \"" << opt << "\" completed successfully.";
    }  

    WARC_LOG_RT_OK << "Option parsing completed successfully.\n";
    return 0; 
}



int MAIN::main( int argc, char* argv[], VOID_DOUBLE_LINK vdl ) {
    int status = this->_parse_opts( argc, argv );
    WARC_ASSERT_RT( status == 0, "Option parsing fault.", status, status );

    status = inet_tls::uplink( {} );
    WARC_ASSERT_RT( status == 0, "Fault at starting the <inet_tls> module.", status, status );

    auto request = _n2yo.tufilin_request( sat::NORAD_ID_NOAA_15, 0, 0, 0, 5 );
    WARC_ASSERT_RT( !request.empty(), "Cannot send an empty request.", -1, -1 );

    _n2yo.socket = inet_tls::BRIDGE::alloc( "158.69.117.9", inet_tls::INET_PORT_HTTPS );
    auto response = _n2yo.socket->xchg( request.c_str(), request.size(), 1000 );
    inet_tls::BRIDGE::free( std::move( _n2yo.socket ) );

    std::cout << response << '\n';

    auto idx = response.find( "\r\n\r\n" ) + 2;
    auto sz = atoi( response.c_str() + idx );

    std::string json{ response.c_str() + idx };
    std::cout << json;

    //std::error_code ec;
    //boost::json::value json_acc = boost::json::parse( json.c_str(), ec );

    status = inet_tls::downlink( {} );
    return status;
}


};
