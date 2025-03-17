#pragma once
/*
*/

#include <NLN/descriptor.hpp>
#include <NLN/comms.hpp>
#include <NLN/assert.hpp>



namespace _ENGINE_NAMESPACE {



enum BIT_END {
    BIT_END_LITTLE,
    BIT_END_BIG
};

class Bytes {
public:
    bool is_char_hex( char c ) {
        c = std::tolower( c );
        return ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' );
    }

    int hex_char2int( char c ) {
        if( c >= '0' && c <= '9' ) return c - '0';
        c = tolower( c );
        if( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
        return -1;
    }

public:
    static std::vector< UBYTE > hex_chars2bytes( std::string_view str ) {
        std::vector< UBYTE > arr; arr.reserve( str.length() >> 1 );
        for( int idx = 0; idx < str.length(); idx += 2 ) {
            UBYTE crt = hex_char2int( str[ idx + 1 ] );
            NLN_ASSERT( crt != -1, {} );

            UBYTE& b = arr.emplace_back( crt );

            crt = hex_char2int( str[ idx ] );
            NLN_ASSERT( crt != -1, {} );

            b |= ( crt << 4 );
        } 
        return arr;
    }

public:
    template< typename T > requires std::is_integral_v< T >
    static T as( char* src, size_t count, BIT_END end ) {
        char dst[ sizeof( T ) ];

        const bool is_negative =
            ( *reinterpret_cast< char* >( src + ( end == BIT_END_LITTLE ? count - 1 : 0 ) ) ) >> 7
            &&
            std::is_signed_v< T >;

        for( size_t n = count; n < sizeof( T ); ++n )
            dst[ n ] = is_negative ? -1 : 0;

        for( size_t n = 0; n < count && n < sizeof( T ); ++n )
            dst[ n ] = src[ end == BIT_END_LITTLE ? n : count - n - 1 ];

        return *reinterpret_cast< T* >( dst );
    }

    template< typename T, size_t count > requires std::is_integral_v< T >
    static T as( char* src, BIT_END end ) {
        if constexpr( count != sizeof( T ) )
            return as< T >( src, count, end );
        else {
            switch( end ) {
                case BIT_END_LITTLE: return *reinterpret_cast< T* >( src );
                case BIT_END_BIG: {
                    char dst[ sizeof( T ) ];

                    for( size_t n = 0; n < count; ++n ) 
                        dst[ n ] = src[ count - n - 1 ];

                    return *reinterpret_cast< T* >( dst );
                }
            }
        }
    }

    template< typename T, BIT_END end > requires std::is_integral_v< T >
    static T as( char* src, size_t count ) {
        char dst[ sizeof( T ) ];

        const bool is_negative =
            ( *reinterpret_cast< char* >( src + ( end == BIT_END_LITTLE ? count - 1 : 0 ) ) ) >> 7
            &&
            std::is_signed_v< T >;

        for( size_t n = count; n < sizeof( T ); ++n )
            dst[ n ] = is_negative ? -1 : 0;

        for( size_t n = 0; n < count && n < sizeof( T ); ++n )
            dst[ n ] = src[ end == BIT_END_LITTLE ? n : count - n - 1 ];

        return *reinterpret_cast< T* >( dst );
    }

    template< typename T, size_t count, BIT_END end > requires std::is_integral_v< T >
    static T as( char* src ) {
        if constexpr( count != sizeof( T ) )
            return as< T, end >( src, count );
        else {
            if constexpr( end == BIT_END_LITTLE )
                return *reinterpret_cast< T* >( src );
            else {
                char dst[ sizeof( T ) ];

                for( size_t n = 0; n < count; ++n ) 
                    dst[ n ] = src[ count - n - 1 ];

                return *reinterpret_cast< T* >( dst );
            }
        }
    }

};



};
