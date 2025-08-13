#pragma once
/**
 * @file command-line-interpreter.hpp
 * @brief Structure keeping references to the majority of the drives and triggering them based on the given text command.
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"
#include "track-drive.hpp"
#include "ring-drive.hpp"

RP_NAMESPACE {


struct COMMAND_LINE_INTERPRETER {
    inline static constexpr int    TOKEN_BUFFER_SIZE   = 6;
    inline static constexpr char   TOKEN_SEPARATOR     = ' ';

    typedef   std::tuple< STATUS, std::string >   exec_status_t;
#define _RP_CLI_ROUTE_FUNC( _func_ ) exec_status_t _func_##_route_tokens( std::string* tokens, int tok_count )
#define _RP_CLI_DIVE_ROUTE( _component_ ) _component_##_route_tokens( tokens + 1, tok_count - 1 )

#define _RP_CLI_ASSERT_TOK_COUNT_GE( _count_, _format_ ) if( tok_count < _count_ ) return { -1, _format_ };
#define _RP_CLI_NO_ROUTE return { -1, "Line no route." };
#define _RP_CLI_OK return { 0, "Line OK." }

#define _RP_CLI_ASSERT_ARG( _cond_, _msg_ ) if( !(_cond_) ) return { -1, (_msg_) };

#define _RP_CLI_HASH_TOK_0 std::hash< std::string >{}( tokens[ 0 ] )

    COMMAND_LINE_INTERPRETER( TRACK_DRIVE* track_drive, RING_DRIVE* ring_drive )
    : _track_drive{ track_drive }, _ring_drive{ ring_drive }
    {}

    TRACK_DRIVE*   _track_drive   = nullptr;
    RING_DRIVE*    _ring_drive    = nullptr;

    exec_status_t exec( std::string_view line ) {
        std::string tokens[ TOKEN_BUFFER_SIZE ];

        int         char_count = 0;
        int         tok_count  = 0;
        const char* tok_begin  = line.data();
        
        for( int char_at = 0; char_at < line.length(); ++char_at ) {
            if( line[ char_at ] == TOKEN_SEPARATOR ) {
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

        return this->_entry_route_tokens( tokens, tok_count );
    }

    _RP_CLI_ROUTE_FUNC( _entry ) { 
        switch( _RP_CLI_HASH_TOK_0 ) {
            case 0x97422bce: return _RP_CLI_DIVE_ROUTE( _hash );
            case 0x3984c0ec: return _RP_CLI_DIVE_ROUTE( _track );
            case 0xbe84067d: return _RP_CLI_DIVE_ROUTE( _ring );
            case 0x160ab82f: return _RP_CLI_DIVE_ROUTE( _restart );
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
            case 0x7bab4f3d: return _RP_CLI_DIVE_ROUTE( _track_test );
        }
        _RP_CLI_NO_ROUTE;
    }

    _RP_CLI_ROUTE_FUNC( _track_test ) { _RP_CLI_ASSERT_TOK_COUNT_GE( 1, "track test <test_number> <args>..." );
        switch( atoi( tokens[ 0 ].c_str() ) ) {
            case 1: {
                _RP_CLI_ASSERT_TOK_COUNT_GE( 3, "track test 1 <power> <duration_ms>" );

                float pwr = atof( tokens[ 1 ].c_str() ); _RP_CLI_ASSERT_ARG( pwr >= 0.0 && pwr <= 1.0, "Track power shall be between 0.0 and 1.0." );
                int   ms  = atoi( tokens[ 2 ].c_str() ); _RP_CLI_ASSERT_ARG( ms >= 0 && ms <= 10000, "Maximum track duration is 10 seconds." );

                _track_drive->test_sequence_1( pwr, ms ); 
                return { 0, "Track test 1 complete." };
            }
        }

        return { -1, "Invalid track test number." };
    }

/**
 * @brief Ring.
 */
    _RP_CLI_ROUTE_FUNC( _ring ) { _RP_CLI_ASSERT_TOK_COUNT_GE( 1, "ring <attention>" );
        return { 0, _ring_drive->attention( tokens[ 0 ] ) };
    }

/**
 * @brief System control.
 */
    _RP_CLI_ROUTE_FUNC( _restart ) { _RP_CLI_ASSERT_TOK_COUNT_GE( 1, "restart <drive>" );
        switch( _RP_CLI_HASH_TOK_0 ) {
            case 0xbe84067d: _ring_drive->restart(); return { 0, "Restarted ring drive." };
        }

        return { -1, "No such drive." };
    }

};


};