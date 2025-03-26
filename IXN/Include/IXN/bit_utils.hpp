#pragma once
/*====== IXT-NLN Engine - Bit Utils - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Misc functionality for bits and bytes manipulation.
|
======*/

#include <IXN/descriptor.hpp>
#include <IXN/comms.hpp>
#include <IXN/assert.hpp>



namespace _ENGINE_NAMESPACE { namespace Bit {



enum End_ {
    End_Little,
    End_Big
};


inline bool is_char_hex( char c ) {
    c = std::tolower( c );
    return ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' );
}

inline int hex_char2int( char c ) {
    if( c >= '0' && c <= '9' ) return c - '0';
    c = tolower( c );
    if( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
    return -1;
}

inline DWORD hex_chars2bytes( UBYTE* dst, std::string_view src, DWORD dst_max = std::numeric_limits< DWORD >::max() ) {
    if( src.empty() || dst_max <= 0 ) return 0;

    DWORD dst_idx = 0;
    DWORD src_idx = 0;

    if( src.length() & 1 ) {
        UBYTE first = hex_char2int( src[ 0 ] );
        IXN_ASSERT( first != -1, -1 );

        dst[ dst_idx++ ] = first;
        src_idx = 1;
    }

    for( ; src_idx < src.length() && dst_idx < dst_max; src_idx += 2 ) {
        UBYTE crt = hex_char2int( src[ src_idx + 1 ] );
        IXN_ASSERT( crt != -1, -1 );

        UBYTE& b = dst[ dst_idx++ ] = crt;

        crt = hex_char2int( src[ src_idx ] );
        IXN_ASSERT( crt != -1, -1 );

        b |= ( crt << 4 );
    } 

    return dst_idx;
}

inline std::vector< UBYTE > hex_chars2bytes( std::string_view src ) {
    std::vector< UBYTE > arr; arr.assign( ( src.length() & 1 ) ? ( ( src.length() + 1 ) >> 1 ) : ( src.length() >> 1 ), 0 );
    if( hex_chars2bytes( arr.data(), src, arr.size() ) < 0 ) return {};
    return arr;
} 

inline DWORD bytes2hex_chars( char* dst, UBYTE* src, DWORD src_count, bool u, DWORD dst_max = std::numeric_limits< DWORD >::max() ) {
    if( src_count <= 0 || dst_max <= 0 ) return 0;

    DWORD dst_idx = 0;
    DWORD src_idx = 0;

    for( ; src_idx < src_count && dst_idx < dst_max; ++src_idx ){
        int crt = src[ src_idx ] & 0x0f;
        dst[ dst_idx + 1 ] = crt <= 9 ? crt + '0' : ( u ? 'A' : 'a' ) + crt - 10;
        crt = src[ src_idx ] >> 4;
        dst[ dst_idx ] = crt <= 9 ? crt + '0' : ( u ? 'A' : 'a' ) + crt - 10;

        dst_idx += 2;
    }

    return dst_idx;
}

inline std::string bytes2hex_chars( UBYTE* src, DWORD src_count, bool u ) {
    std::string str; str.assign( src_count << 1,'\0' );
    bytes2hex_chars( str.data(), src, src_count, u, str.length() );
    return str;
}




template< typename T > requires std::is_integral_v< T >
inline T as( char* src, size_t count, End_ end ) {
    char dst[ sizeof( T ) ];

    const bool is_negative =
        ( *reinterpret_cast< char* >( src + ( end == End_Little ? count - 1 : 0 ) ) ) >> 7
        &&
        std::is_signed_v< T >;

    for( size_t n = count; n < sizeof( T ); ++n )
        dst[ n ] = is_negative ? -1 : 0;

    for( size_t n = 0; n < count && n < sizeof( T ); ++n )
        dst[ n ] = src[ end == End_Little ? n : count - n - 1 ];

    return *reinterpret_cast< T* >( dst );
}

template< typename T, size_t count > requires std::is_integral_v< T >
inline T as( char* src, End_ end ) {
    if constexpr( count != sizeof( T ) )
        return as< T >( src, count, end );
    else {
        switch( end ) {
            case End_Little: return *reinterpret_cast< T* >( src );
            case End_Big: {
                char dst[ sizeof( T ) ];

                for( size_t n = 0; n < count; ++n ) 
                    dst[ n ] = src[ count - n - 1 ];

                return *reinterpret_cast< T* >( dst );
            }
        }
    }
}

template< typename T, End_ end > requires std::is_integral_v< T >
inline T as( char* src, size_t count ) {
    char dst[ sizeof( T ) ];

    const bool is_negative =
        ( *reinterpret_cast< char* >( src + ( end == End_Little ? count - 1 : 0 ) ) ) >> 7
        &&
        std::is_signed_v< T >;

    for( size_t n = count; n < sizeof( T ); ++n )
        dst[ n ] = is_negative ? -1 : 0;

    for( size_t n = 0; n < count && n < sizeof( T ); ++n )
        dst[ n ] = src[ end == End_Little ? n : count - n - 1 ];

    return *reinterpret_cast< T* >( dst );
}

template< typename T, size_t count, End_ end > requires std::is_integral_v< T >
inline T as( char* src ) {
    if constexpr( count != sizeof( T ) )
        return as< T, end >( src, count );
    else {
        if constexpr( end == End_Little )
            return *reinterpret_cast< T* >( src );
        else {
            char dst[ sizeof( T ) ];

            for( size_t n = 0; n < count; ++n ) 
                dst[ n ] = src[ count - n - 1 ];

            return *reinterpret_cast< T* >( dst );
        }
    }
}



}; };
