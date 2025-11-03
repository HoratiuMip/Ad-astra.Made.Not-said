#pragma once
/**
 * @file: cli.hpp
 * @brief: Structure keeping references to the majority of the drives and triggering them based on the given text command.
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"
#include "track.hpp"
#include "lights.hpp"

namespace rp {


class Cli_drive {
public:
    const struct PIN_MAP {
    } _pin_map;

public:
    inline static constexpr int   TOKEN_BUFFER_SIZE   = 6;
    inline static const char*     TOKEN_SEPARATORS    = " \n";

    typedef   std::tuple< rnk::status_t, std::string >   exec_status_t;
#define _RP_CLI_ROUTE_FUNC( _func_ ) exec_status_t _func_##_route_tokens( std::string* tokens, int tok_count )
#define _RP_CLI_DIVE_ROUTE( _component_ ) _component_##_route_tokens( tokens + 1, tok_count - 1 )

#define _RP_CLI_ASSERT_TOK_COUNT_GE( _count_, _format_ ) if( tok_count < _count_ ) return { -0x1, _format_ };
#define _RP_CLI_NO_ROUTE return { -0x1, "Invalid route." };
#define _RP_CLI_OK return { 0x0, "Route OK." }

#define _RP_CLI_ASSERT_ARG( _cond_, _msg_ ) if( !(_cond_) ) return { -1, (_msg_) };

#define _RP_CLI_HASH_TOK_0 std::hash< std::string >{}( tokens[ 0 ] )

public:
    Cli_drive( const PIN_MAP& pin_map_, Track_drive* track_drive_, Light_drive* light_drive_ )
    : _pin_map{ pin_map_ }, _track_drive{ track_drive_ }, _light_drive{ light_drive_ }
    {}

RNK_PROTECTED:
    Track_drive*   _track_drive   = nullptr;
    Light_drive*   _light_drive   = nullptr;

public:
    exec_status_t exec( std::string_view line ) {
        std::string tokens[ TOKEN_BUFFER_SIZE ];

        int         char_count = 0;
        int         tok_count  = 0;
        const char* tok_begin  = line.data();
        
        for( int char_at = 0; char_at < line.length(); ++char_at ) {
            if( nullptr != strchr( TOKEN_SEPARATORS, line[ char_at ] ) ) {
                if( char_count == 0 ) {
                    tok_begin = &line[ char_at ] + 1;
                    continue;
                }

                tokens[ tok_count++ ] = std::string{ tok_begin, char_count };
                tok_begin = &line[ char_at ] + 1;
                char_count = 0;
                continue;
            }

            ++char_count;
        }

        if( char_count > 0 ) tokens[ tok_count++ ] = std::string{ tok_begin, char_count };
        
        _RP_CLI_ASSERT_TOK_COUNT_GE( 1, "<line>" );
        
        exec_status_t status = this->_entry_route_tokens( tokens, tok_count );
        std::get< std::string >( status ) += "\r\n";
        return status;
    }

RNK_PROTECTED:
    _RP_CLI_ROUTE_FUNC( _entry ) { 
        switch( _RP_CLI_HASH_TOK_0 ) {
            case 0x97422bce: return _RP_CLI_DIVE_ROUTE( _hash );
            case 0x3984c0ec: return _RP_CLI_DIVE_ROUTE( _track );
            // case 0xbe84067d: return _RP_CLI_DIVE_ROUTE( _ring );
            // case 0x160ab82f: return _RP_CLI_DIVE_ROUTE( _restart );
        }

        _RP_CLI_NO_ROUTE;
    }

/**
 * @brief Hash.
 */
    _RP_CLI_ROUTE_FUNC( _hash ) { _RP_CLI_ASSERT_TOK_COUNT_GE( 1, "hash <string>" );
        return { 0, String{ _RP_CLI_HASH_TOK_0, HEX }.c_str() };
    }

/**
 * @brief Track.
 */
    _RP_CLI_ROUTE_FUNC( _track ) { _RP_CLI_ASSERT_TOK_COUNT_GE( 1, "track <command> <args>..." );
        switch( _RP_CLI_HASH_TOK_0 ) {
            case 0x974cc8a8: return _RP_CLI_DIVE_ROUTE( _track_seq );
        }
        _RP_CLI_NO_ROUTE;
    }

    _RP_CLI_ROUTE_FUNC( _track_seq ) { _RP_CLI_ASSERT_TOK_COUNT_GE( 3, "track seq <seq_number> <power> <duration_ms>"  );
        float pwr = atof( tokens[ 1 ].c_str() ); _RP_CLI_ASSERT_ARG( pwr >= 0.0 && pwr <= 1.0, "Track power shall be between 0.0 and 1.0." );
        int   ms  = atoi( tokens[ 2 ].c_str() ); _RP_CLI_ASSERT_ARG( ms >= 0 && ms <= 10000, "Maximum track sequence duration is 10 seconds." );

        switch( atoi( tokens[ 0 ].c_str() ) ) {
            case 1: _track_drive->seq_1( pwr, ms ); return { 0x0, "Track sequence 1 complete." };
            case 2: _track_drive->seq_2( pwr, ms ); return { 0x0, "Track sequence 2 complete." };
            case 3: _track_drive->seq_3( pwr, ms ); return { 0x0, "Track sequence 3 complete." };
            case 4: _track_drive->seq_4( pwr, ms ); return { 0x0, "Track sequence 4 complete." };
            case 5: _track_drive->seq_5( pwr, ms ); return { 0x0, "Track sequence 5 complete." };
        }

        _RP_CLI_ASSERT_ARG( false, "Invalid track sequence number." );
    }

// /**
//  * @brief Ring.
//  */
//     _RP_CLI_ROUTE_FUNC( _ring ) { _RP_CLI_ASSERT_TOK_COUNT_GE( 1, "ring <attention>" );
//         return { 0, _ring_drive->attention( tokens[ 0 ] ) };
//     }

// /**
//  * @brief System control.
//  */
//     _RP_CLI_ROUTE_FUNC( _restart ) { _RP_CLI_ASSERT_TOK_COUNT_GE( 1, "restart <drive>" );
//         switch( _RP_CLI_HASH_TOK_0 ) {
//             case 0xbe84067d: _ring_drive->restart(); return { 0, "Restarted ring drive." };
//         }

//         return { -1, "No such drive." };
//     }

};


};