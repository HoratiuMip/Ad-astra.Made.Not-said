/**
 * @file: BRp/IO_string_utils.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/IO_string_utils.hpp>

namespace A113 { namespace BRp {


A113_BR_FNC void ipv4_addr_str_t::make_zero( void ) {
    strcpy( buf, "0.0.0.0" );
}

A113_BR_FNC void ipv4_addr_str_t::from_ptr( uint32_t addr, ipv4_addr_str_t* ptr ) {
    int n = 0x0;
#ifdef A113_TARGET_END_BIG
    for( int bi = 0x3; bi >= 0x0; --bi )
#else
    for( int bi = 0x0; bi < 0x4; ++bi )  
#endif
    {
        unsigned char b = ( ( unsigned char* )&addr )[ bi ];
        n += snprintf( ptr->buf + n, 0x4, "%u", b );
        *( ptr->buf + n ) = '.';
        ++n;
    }
    ptr->buf[ n - 1 ] = ptr->buf[ BUF_SIZE - 1 ] = '\0';
}

A113_BR_FNC ipv4_addr_str_t ipv4_addr_str_t::from( uint32_t addr ) {
    ipv4_addr_str_t res = {};
    ipv4_addr_str_t::from_ptr( addr, &res );
    return res;
} 

A113_BR_FNC uint32_t ipv4_addr_str_t::from( const char* addr_str ) {
    char aux[ BUF_SIZE ] = { '\0' }; strncpy( aux, addr_str, BUF_SIZE - 1 );

    uint32_t result = 0x0;
    char*    head   = aux;

    for( int bi = 0x0; bi < 0x4; ++bi ) {
        char* dot = strchr( head, '.' );
        if( nullptr == dot ) {
            if( 0x3 == bi ) goto l_last;
            else return 0x0;
        }

        *dot = '\0';
    l_last:
        result |= ( ( uint8_t )atoi( head ) ) << ( 0x8*bi ) ; 
 
        head = dot + 1;
    }
 
    return result;
}


} };
