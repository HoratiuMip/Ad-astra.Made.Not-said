#pragma once
/*====== IXT-NLN uC Engine - Common utils - Vatca "Mipsan" Tudor-Horatiu
|
>
|
======*/


namespace ixN { namespace uC {


enum LogLevel_ {
    LogLevel_Ok, LogLevel_Warning, LogLevel_Error, LogLevel_Critical, LogLevel_Info
};
const char* LogLevel_strings[] = { "[ Ok ] ", "[ Warning ] ", "[ Error ] ", "[ Critical ] ", "[ Info ] " };
inline static struct __PRINTF {
    template< typename ...Args >
    inline void operator () ( const char* fmt_str, Args&&... args ) {
        static constexpr int BUF_MAX_SIZE = 256;
        char buffer[ BUF_MAX_SIZE ];
        sprintf( buffer, fmt_str, std::forward< Args >( args )... );
        Serial.print( buffer );
    }

    template< typename ...Args >
    inline void operator () ( LogLevel_ level, const char* fmt_str, Args&&... args ) {
        Serial.print( LogLevel_strings[ ( int )level ] );
        ( *this )( fmt_str, std::forward< Args >( args )... );
    }

    template< typename ...Args >
    inline void operator () ( LogLevel_ level, const char* who, const char* fmt_str, Args&&... args ) {
        Serial.print( LogLevel_strings[ ( int )level ] );
        if( who != nullptr ) ( *this )( "[ %s ] ", who );
        ( *this )( fmt_str, std::forward< Args >( args )... );
    }

    inline void _buf2serial( char* buffer ) {
        
    }

} _printf;


} };
