#include <warc/warc-main.hpp>

namespace warc {


static struct _INTERNAL {
    struct OPT {
        enum USABLE : IXT::BYTE {
            USABLE_CMDL_LAUNCH_MSK    = 0b001,
            USABLE_CONFIG_FILE_MSK    = 0b010,
            USABLE_CMDL_REAL_TIME_MSK = 0b100
        };

        int           idx;
        const char*   name;
        /*
        | bit 0 - from command line on launch
        | bit 1 - from configuration file
        | bit 2 - from real time command line
        */
        IXT::BYTE     usable;
        int           least_argc;
        int           ( MAIN::*proc )( int, char**, const char* );

    } opts[ 7 ] = {
        { 1, "--from-config",         0b001, 1, &MAIN::_parse_proc_from_config },
        { 2, "--n2yo-api-key",        0b111, 1, &MAIN::_parse_proc_n2yo_api_key },
        { 3, "--n2yo-ip",             0b111, 1, &MAIN::_parse_proc_n2yo_ip },
        { 4, "--n2yo-bulk-count",     0b111, 1, &MAIN::_parse_proc_n2yo_bulk_count },
        { 5, "--n2yo-mode",           0b111, 1, &MAIN::_parse_proc_n2yo_mode },
        { 6, "--earth-imm",           0b101, 0, &MAIN::_parse_proc_earth_imm },
        { 7, "--earth-imm-lens-sens", 0b111, 1, &MAIN::_parse_proc_earth_imm_lens_sens }
    };
    const int optc = sizeof( opts ) / sizeof( OPT );

    struct CONFIG {
        inline static constexpr int N2YO_BULK_COUNT_MAX = 7200;
        enum N2YO_MODE : IXT::DWORD {
            N2YO_MODE_UNKNWN = 0,
            N2YO_MODE_RAND   = 1,
            N2YO_MODE_PAST   = 2,
            N2YO_MODE_REAL   = 3,

            _N2YO_MODE_FORCE_DWORD = 0x7f'ff'ff'ff
        };

        N2YO_MODE     n2yo_mode             = N2YO_MODE_RAND;
        std::string   n2yo_ip               = ""; 
        int           n2yo_bulk_count       = 180;

        bool          earth_imm             = false;
        float         earth_imm_lens_sens   = 1.0;

    } config;

} _internal;


WARC_MAIN_PARSE_PROC_FUNC( MAIN::_parse_proc_n2yo_api_key ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_n2yo_api_key()" );

    WARC_ASSERT_RT( argv != nullptr, "Argv is NULL.", -1, -1 );
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
        WARC_LOG_RT_INTEL << "This process, \"" << process << "\", has no burnt key.";
    else
        WARC_LOG_RT_INTEL << "This process, \"" << process << "\", has the key \"" << key << "\" burnt.";

    return 0;
}

}

WARC_MAIN_PARSE_PROC_FUNC( MAIN::_parse_proc_n2yo_ip ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_n2yo_ip()" );

    WARC_ASSERT_RT( argv != nullptr, "Argv is NULL.", -1, -1 );
    WARC_ASSERT_RT( argv[ 0 ] != nullptr, "Address is NULL.", -1, -1 );

    _internal.config.n2yo_ip = argv[ 0 ];

    WARC_LOG_RT_OK << "N2YO ip: \"" << _internal.config.n2yo_ip << "\".";
    return 0;
}

WARC_MAIN_PARSE_PROC_FUNC( MAIN::_parse_proc_n2yo_bulk_count ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_n2yo_bulk_count()" );

    WARC_ASSERT_RT( argv != nullptr, "Argv is NULL.", -1, -1 );
    WARC_ASSERT_RT( argv[ 0 ] != nullptr, "Bulk count is NULL.", -1, -1 );

    int bulk_count = atoi( argv[ 0 ] );
    WARC_ASSERT_RT( bulk_count >= 0 && bulk_count <= _INTERNAL::CONFIG::N2YO_BULK_COUNT_MAX, "Bulk count out of bounds.", bulk_count, -1 );
    _internal.config.n2yo_bulk_count = bulk_count;

    WARC_LOG_RT_OK << "N2YO bulk count: \"" << _internal.config.n2yo_bulk_count << "\".";
    return 0;
}

WARC_MAIN_PARSE_PROC_FUNC( MAIN::_parse_proc_n2yo_mode ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_n2yo_mode()" );

    WARC_ASSERT_RT( argv != nullptr, "Argv is NULL.", -1, -1 );
    WARC_ASSERT_RT( argv[ 0 ] != nullptr, "Mode is NULL.", -1, -1 );

    const char* mode_strs[] = {
        "rand", "past", "real"
    };

    const char** match = std::find_if( mode_strs, mode_strs + sizeof( mode_strs ) / sizeof( void* ), [ & ] ( const char* cmp ) -> bool {
        return strcmp( argv[ 0 ], cmp ) == 0;
    } );

    switch( ptrdiff_t diff = match - mode_strs; diff ) {
        case 0: case 1: case 2: {
            _internal.config.n2yo_mode = ( _INTERNAL::CONFIG::N2YO_MODE )( ( ptrdiff_t )_INTERNAL::CONFIG::N2YO_MODE_RAND + diff );
            break;
        };

        default: {
            _internal.config.n2yo_mode = _INTERNAL::CONFIG::N2YO_MODE_UNKNWN;
            WARC_LOG_RT_ERROR << "Unknown N2YO mode: \"" << argv[ 0 ] << "\".";
            return -1;
        }
    }

    WARC_LOG_RT_OK << "N2YO mode: \"" << argv[ 0 ] << "\".";
    return 0;
}

WARC_MAIN_PARSE_PROC_FUNC( MAIN::_parse_proc_earth_imm ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_earth_imm()" );

    _internal.config.earth_imm = true;
    WARC_LOG_RT_OK << "Enabled earth immersion module.";
    return 0;
}

WARC_MAIN_PARSE_PROC_FUNC( MAIN::_parse_proc_earth_imm_lens_sens ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_earth_imm_lens_sens()" );

    WARC_ASSERT_RT( argv != nullptr, "Argv is NULL.", -1, -1 );
    WARC_ASSERT_RT( argv[ 0 ] != nullptr, "Sens is NULL.", -1, -1 );

    float lens_sens = atof( argv[ 0 ] );
    _internal.config.earth_imm_lens_sens = lens_sens;

    WARC_LOG_RT_OK << "Earth immersion lens sensitivity: \"" << _internal.config.earth_imm_lens_sens << "\".";
    return 0;
}


WARC_MAIN_PARSE_PROC_FUNC( MAIN::_parse_proc_from_config ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_proc_from_config()" );

    WARC_ASSERT_RT( argv != nullptr, "Argv is NULL.", -1, -1 );
    WARC_ASSERT_RT( argv[ 0 ] != nullptr, "Configuration file path is NULL.", -1, -1 );

    WARC_LOG_RT_INTEL << "Parsing json.";

    std::ifstream file{ argv[ 0 ] };
    WARC_ASSERT_RT( file, "Could not open configuration file.", argv[ 0 ], -1 );

    size_t sz = IXT::File::byte_count( file );
    IXT::UPtr< char[] > buffer{ ( char* )malloc( sz + 1 ) };
    file.read( buffer.get(), sz );
    file.close();
    buffer[ sz ] = 0;

    std::error_code ec;
    boost::json::value jv = boost::json::parse( buffer.get(), ec );
    WARC_ASSERT_RT( ec.value() == 0, "Fault when parsing json.", ec.message(), -1 );

    auto* config = jv.if_object();
    WARC_ASSERT_RT( config != nullptr, "Configuration is not a JSON object.", -1, -1 );

    for( auto& opt : _internal.opts ) {
        if( ( opt.usable & _INTERNAL::OPT::USABLE_CONFIG_FILE_MSK ) == 0 ) continue;

        boost::json::value& v = ( *config )[ opt.name ];

        if( v.is_null() ) {
            //WARC_LOG_RT_WARNING << "Configuration \"" << opt.name << "\" is null. Continuing...";
            continue; 
        }

        auto* str = v.if_string();
        if( str == nullptr ) {
            WARC_LOG_RT_WARNING << "Configuration \"" << opt.name << "\" is not a string. Continuing...";
            continue; 
        }

        WARC_LOG_RT_INTEL << "Proc configuration \"" << opt.name << "\".";

        std::vector< std::string > arg_strs; arg_strs.reserve( 16 );
        std::vector< char* >       args; args.reserve( 16 );

        size_t pos1 = 0;
        size_t pos2;
        do {
            pos2 = str->find( ' ', pos2 ); pos2 = ( pos2 == std::string::npos ? str->size() : pos2 );

            arg_strs.emplace_back( str->c_str() + pos1, pos2 - pos1 );
            args.emplace_back( arg_strs.back().data() );
            
            while( pos2 < str->size() && ( *str )[ pos2 ] == ' ' )
                ++pos2;
            pos1 = pos2;

        } while( pos2 < str->size() );

        int status = ( this->*opt.proc )( ( int )args.size(), args.data(), process );
        WARC_ASSERT_RT( status == 0, "Proc configuration fault.", status, -1 );
    }

    WARC_LOG_RT_OK << "Json parsed.";
    return 0;
}

int MAIN::_parse_opts( int argc, char* argv[] ) {
    _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::_parse_opts()" );

    WARC_LOG_RT_INTEL << "Begin option parsing."; 

    for( int idx = 1; idx < argc; ++idx ) {
        char*   opt      = argv[ idx ];
        char**  opt_ptr  = argv + idx;

        if( !std::string_view{ opt }.starts_with( "--" ) ) continue;

        _INTERNAL::OPT* record = std::find_if( _internal.opts, _internal.opts + _internal.optc, [ &opt ] ( _INTERNAL::OPT& rec ) -> bool {
            if( ( rec.usable & _INTERNAL::OPT::USABLE_CMDL_LAUNCH_MSK ) == 0 ) return false;
            return strcmp( opt, rec.name ) == 0;
        } );

        if( record == _internal.opts + _internal.optc ) {
            WARC_LOG_RT_WARNING << "Ignoring invalid option \"" << opt << "\".";
            continue;
        }

        int argc_step = 1;
        if( 
            ( idx + 1 + record->least_argc > argc ) 
            ||
            ( [ &idx, &argv, &argc, &record, &argc_step ] () mutable -> bool { 
                for( ; idx + 1 < argc; ++argc_step, ++idx )
                    if( std::string_view{ argv[ idx + 1 ] }.starts_with( "--" ) )
                        break;

                return argc_step < record->least_argc;
            } )()
        ) {
            WARC_LOG_RT_ERROR << "Not enough arguments for option \"" << opt << "\", at least (" << record->least_argc << ") required.\n";
            return -1;
        }

        WARC_LOG_RT_INTEL << "Begin proc for \"" << opt << "\".";

        int status = ( this->*( record->proc ) )( argc_step, opt_ptr + 1, argv[ 0 ] );
        WARC_ASSERT_RT( status == 0, "Option proc exited with error.\n", status, status );

        WARC_LOG_RT_OK << "Proc for \"" << opt << "\" complete.";
    }  

    WARC_LOG_RT_OK << "Option parsing complete.\n";
    return 0; 
}


int MAIN::main( int argc, char* argv[] ) {
    int status = this->_parse_opts( argc, argv );
    WARC_ASSERT_RT( status == 0, "Option parsing fault.", status, status );

    status = IXT::initial_uplink( argc, argv, 0, nullptr, nullptr );
    WARC_ASSERT_RT( status == 0, "Fault at starting the IXT engine.", status, status );

    status = inet_tls::uplink();
    WARC_ASSERT_RT( status == 0 || WARC_INET_TLS == 0, "Fault at starting the <inet_tls> module.", status, status );

    char continue_program;
    do {
        WARC_LOG_RT_OK << "Initialization complete. Continue the program? [Y/N]: ";
        std::cin >> continue_program;
        if( continue_program == 'N' ) goto l_main_end;
    } while( continue_program != 'Y' );

    if( _internal.config.earth_imm ) {
        this->_earth = std::make_shared< imm::EARTH >();

        this->_earth->lens_sens = _internal.config.earth_imm_lens_sens;

        this->_earth->set_sat_pos_update_func( [ &, this ] ( sat::NORAD_ID norad_id, std::deque< sat::POSITION >& positions ) -> imm::EARTH_SAT_UPDATE_RESULT {
            _WARC_IXT_COMPONENT_DESCRIPTOR( WARC_MAIN_STR"::lambda::sat_updater()" );
            
            WARC_LOG_RT_INTEL << "Updating satellite #" << ( int )norad_id << ", mode #" << _internal.config.n2yo_mode << ".";

            switch( _internal.config.n2yo_mode ) {
                case _INTERNAL::CONFIG::N2YO_MODE_RAND: {
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
                break; }

                case _INTERNAL::CONFIG::N2YO_MODE_PAST: {
                    std::filesystem::path path{ WARC_RUPTURE_DATA_ROOT_DIR };
                    path /= "n2yo_past_data_";
                    path += std::to_string( ( int )norad_id );
                    path += ".json";

                    std::ifstream file{ path, std::ios_base::binary };

                    WARC_ASSERT_RT( file, "Could not open n2yo past data file.", -1, imm::EARTH_SAT_UPDATE_RESULT_HOLD );

                    size_t sz = IXT::File::byte_count( file );
                    IXT::UPtr< char[] > buffer{ ( char* )malloc( sz + 1 ) };
                    file.read( buffer.get(), sz );
                    buffer[ sz ] = 0;

                    auto res = this->_n2yo.json_2_positions( buffer.get() );
                    WARC_ASSERT_RT( !res.data.empty(), "Empty positions deque.", -1, imm::EARTH_SAT_UPDATE_RESULT_HOLD );
                    positions.assign( res.data.begin(), res.data.end() );
                break; }

                case _INTERNAL::CONFIG::N2YO_MODE_REAL: {
                    auto res = this->_n2yo.quick_position_xchg( _internal.config.n2yo_ip.c_str(), norad_id, _internal.config.n2yo_bulk_count );
                    WARC_ASSERT_RT( !res.data.empty(), "Empty positions deque.", -1, imm::EARTH_SAT_UPDATE_RESULT_HOLD );
                    positions.assign( res.data.begin(), res.data.end() );
                break; }
            }

            return imm::EARTH_SAT_UPDATE_RESULT_OK;
        } );

        this->_earth->main( argc, argv );
    }

l_main_end:
    status = inet_tls::downlink();
    status = IXT::final_downlink( argc, argv, 0, nullptr, nullptr );
    return status;
}


};
