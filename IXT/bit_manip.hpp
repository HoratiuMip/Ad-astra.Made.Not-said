#ifndef _ENGINE_BIT_MANIP_HPP
#define _ENGINE_BIT_MANIP_HPP
/*
*/

#include "descriptor.hpp"



namespace _ENGINE_NAMEPACE {



enum BIT_END {
    BIT_END_LITTLE,
    BIT_END_BIG
};

class Bytes {
public:
    template< typename T >
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

    template< typename T, size_t count >
    static T as( char* src, BIT_END end ) {
        
    }

    template< typename T, BIT_END end >
    static T as( char* src, size_t count ) {

    }

    template< typename T, size_t count, BIT_END end >
    static T as( char* src ) {
        
    }

};



};



#endif