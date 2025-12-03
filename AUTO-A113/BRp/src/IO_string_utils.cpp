/**
 * @file: BRp/IO_string_utils.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/brp/IO_string_utils.hpp>

namespace a113 { namespace io {


A113_IMPL_FNC void ipv4_addr_str_t::make_zero( void ) {
    strcpy( buf, "0.0.0.0" );
}

A113_IMPL_FNC void ipv4_addr_str_t::from_ptr( ipv4_addr_t addr_, ipv4_addr_str_t* ptr_ ) {
    int n = 0x0;
#ifdef A113_TARGET_END_BIG
    for( int bi = 0x3; bi >= 0x0; --bi )
#else
    for( int bi = 0x0; bi < 0x4; ++bi )  
#endif
    {
        unsigned char b = ( ( unsigned char* )&addr_ )[ bi ];
        n += snprintf( ptr_->buf + n, 0x4, "%u", b );
        *( ptr_->buf + n ) = '.';
        ++n;
    }
    ptr_->buf[ n - 1 ] = ptr_->buf[ BUF_SIZE - 1 ] = '\0';
}

A113_IMPL_FNC ipv4_addr_str_t ipv4_addr_str_t::from( ipv4_addr_t addr_ ) {
    ipv4_addr_str_t res = {};
    ipv4_addr_str_t::from_ptr( addr_, &res );
    return res;
} 

A113_IMPL_FNC ipv4_addr_t ipv4_addr_str_t::from( const char* addr_str_ ) {
    char aux[ BUF_SIZE ] = { '\0' }; strncpy( aux, addr_str_, BUF_SIZE - 1 );

    ipv4_addr_t result = 0x0;
    char*       head   = aux;

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
