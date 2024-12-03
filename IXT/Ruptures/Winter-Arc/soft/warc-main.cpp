#include <warc/warc-main.hpp>

namespace warc {


static struct _INTERNAL {
    struct CONFIG {
        bool          n2yo_bypass        = false;
        std::string   n2yo_ip           = ""; 
        int64_t       n2yo_bulk_count   = 180;

        bool          earth_imm         = false;
        float         earth_imm_ls      = 1.0;

    }   config;

} _internal;

int MAIN::_parse_proc_from_config( char* argv[], const char* process ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_from_config()" );

    WARC_ASSERT_RT( argv != nullptr, "Argument array is NULL.", -1, -1 );
    WARC_ASSERT_RT( argv[ 0 ] != nullptr, "Configuration file path is NULL.", -1, -1 );

    WARC_LOG_RT_INTEL << "Parsing json.";

    std::ifstream file{ argv[ 0 ] };
    WARC_ASSERT_RT( file, "Configuration file does not exist.", -1, -1 );

    size_t sz = IXT::File::byte_count( file );
    IXT::UPtr< char[] > buffer{ ( char* )malloc( sz + 1 ) };
    file.read( buffer.get(), sz );
    file.close();
    buffer[ sz ] = 0;

    std::error_code ec;
    boost::json::value jv = boost::json::parse( buffer.get(), ec );
    WARC_ASSERT_RT( ec.value() == 0, "Fault when parsing json.", ec.message(), -1 );

    auto& config = jv.as_object();

    struct _PROC {
        const char*                                     str;
        std::function< int( boost::json::value& v ) >   func;

    } procs[] = {
        "n2yo_ip", [] ( boost::json::value& v ) -> int { 
            if( auto* str = v.if_string(); str ) {
                _internal.config.n2yo_ip = *str;
                return 0;
            }
            return -1;
        },
        "n2yo_api_key", [ this ] ( boost::json::value& v ) -> int {
            if( auto* str = v.if_string(); str ) {
                this->_n2yo.api_key = *str;
                return 0;
            }
            return -1;
        },
        "n2yo_bulk_count", [ this ] ( boost::json::value& v ) -> int {
            if( auto* n = v.if_int64(); n ) {
                _internal.config.n2yo_bulk_count = *n;
                return 0;
            }
            return -1;
        },
        "earth_imm_lens_sens", [ this ] ( boost::json::value& v ) -> int {
            if( auto* f = v.if_double(); f ) {
                _internal.config.earth_imm_ls = *f;
                return 0;
            }
            return -1;
        }
    };

    for( auto& proc : procs ) {
        boost::json::value& v = config[ proc.str ];

        if( v.is_null() ) {
            WARC_LOG_RT_WARNING << "Configuration \"" << proc.str << "\" is null. Continuing...";
            continue; 
        }

        WARC_LOG_RT_INTEL << "Setting configuration \"" << proc.str << "\".";
        int status = proc.func( v );
        WARC_ASSERT_RT( status == 0, "Configuration fault.", status, -1 );
        WARC_LOG_RT_OK << "Configuration \"" << proc.str << "\" set.";
    }

    WARC_LOG_RT_OK << "Json parsed.";
    return 0;
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

int MAIN::_parse_proc_n2yo_bypass( char* argv[], const char* process ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_n2yo_bypass()" );

    _internal.config.n2yo_bypass = true;
    WARC_LOG_RT_OK << "N2YO requests will be bypassed.";
    return 0;
}

int MAIN::_parse_proc_earth_imm( char* argv[], const char* process ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_earth_imm()" );

    _internal.config.earth_imm = true;
    WARC_LOG_RT_OK << "Enabled earth immersion module.";
    return 0;
}

int MAIN::_parse_opts( int argc, char* argv[] ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_opts()" );

    int status = 0;

    WARC_LOG_RT_INTEL << "Starting option parsing."; 

    struct _OPT {
        const char*   str;
        int           argc;
        int           ( MAIN::*proc )( char**, const char* );
    } opts[] = {
        { "--from-config", 1, &_parse_proc_from_config },
        { "--n2yo-api-key", 2, &_parse_proc_n2yo_api_key },
        { "--n2yo-bypass", 0, &_parse_proc_n2yo_bypass },
        { "--earth-imm", 0, &_parse_proc_earth_imm },
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
            status -= 1;
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
            status -= 1;
            continue;
        }

        WARC_LOG_RT_INTEL << "Executing procedure for option \"" << opt << "\".";

        int status = ( this->*( record->proc ) )( opt_ptr + 1, argv[ 0 ] );
        WARC_ASSERT_RT( status == 0, "Option procedure exited abnormally.\n", status, status );

        WARC_LOG_RT_OK << "Procedure for option \"" << opt << "\" completed successfully.";
    }  

    if( status == 0 )
        WARC_LOG_RT_OK << "Option parsing complete.\n";
    else
        WARC_LOG_RT_WARNING << "Option parsing incomplete.\n";
    return status; 
}



int MAIN::main( int argc, char* argv[] ) {
    int status = this->_parse_opts( argc, argv );
    WARC_ASSERT_RT( status == 0, "Option parsing fault.", status, status );

    status = IXT::initial_uplink( argc, argv, 0, nullptr, nullptr );
    WARC_ASSERT_RT( status == 0, "Fault at starting the IXT engine.", status, status );

    status = inet_tls::uplink();
    WARC_ASSERT_RT( status == 0, "Fault at starting the <inet_tls> module.", status, status );

    if( _internal.config.earth_imm ) {
        this->_earth = std::make_shared< imm::EARTH >();

        this->_earth->lens_sens = _internal.config.earth_imm_ls;

        this->_earth->set_sat_pos_update_func( [ &, this ] ( sat::NORAD_ID norad_id, std::deque< sat::POSITION >& positions ) -> imm::EARTH_SAT_UPDATE_RESULT {
            if( _internal.config.n2yo_bypass ) {
                positions.emplace_back( sat::POSITION{ 
                    satlatitude:  ( double )( rand() % 180 - 90 ),
                    satlongitude: ( double )( rand() % 360 ),
                    sataltitude:  0,
                    azimuth:      0,
                    elevation:    0,
                    ra:           0,
                    dec:          0,
                    timestamp:    time( nullptr ),
                    eclipsed:     false
                } );
                
                goto l_ok;
            }

            {
            auto res = this->_n2yo.quick_position_xchg( _internal.config.n2yo_ip.c_str(), norad_id, _internal.config.n2yo_bulk_count );

            if( res.data.empty() ) return imm::EARTH_SAT_UPDATE_RESULT_HOLD;
            
            positions.assign( res.data.begin(), res.data.end() );
            }
        
        l_ok:
            return imm::EARTH_SAT_UPDATE_RESULT_OK;
        } );

        this->_earth->main( argc, argv );
    }

    status = inet_tls::downlink();
    status = IXT::final_downlink( argc, argv, 0, nullptr, nullptr );
    return status;
}


};
