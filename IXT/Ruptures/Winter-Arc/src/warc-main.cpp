#include <warc/warc-main.hpp>

namespace warc {


static struct _INTERNAL {
    struct OPTIONS {
        bool   earth_imm   = false;

    }   opts;

} _internal;


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

int MAIN::_parse_proc_earth_imm( char* argv[], const char* process ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_earth_imm()" );

    _internal.opts.earth_imm = true;
    WARC_LOG_RT_OK << "Enabled earth immersion module.";

    return 0;
}

int MAIN::_parse_opts( int argc, char* argv[] ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_opts()" );

    WARC_LOG_RT_INTEL << "Starting option parsing."; 

    struct _OPT {
        const char*   str;
        int           argc;
        int           ( MAIN::*proc )( char**, const char* );
    } opts[] = {
        { "--n2yo-api-key", 2, &_parse_proc_n2yo_api_key },
        { "--earth-imm", 0, &_parse_proc_earth_imm }
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

    status = IXT::initial_uplink( argc, argv, 0, nullptr, nullptr );
    WARC_ASSERT_RT( status == 0, "Fault at starting the IXT engine.", status, status );

    status = inet_tls::uplink( {} );
    WARC_ASSERT_RT( status == 0, "Fault at starting the <inet_tls> module.", status, status );


    if( _internal.opts.earth_imm ) {
        this->_earth = std::make_shared< imm::EARTH >();
        return this->_earth->main( argc, argv );
    }

    this->_n2yo.json_2_positions( n2yo::POSITIONS::JSON_SAMPLE );

    return 0;


    _n2yo.socket = inet_tls::BRIDGE::alloc( "158.69.117.9", inet_tls::INET_PORT_HTTPS );
    auto response = _n2yo.send_get_positions( sat::NORAD_ID_NOAA_15, 0, 0, 0, 5 );
    inet_tls::BRIDGE::free( std::move( _n2yo.socket ) );

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
