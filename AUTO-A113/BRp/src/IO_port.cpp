/**
 * @file: BRp/IO_port.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/IO_port.hpp>

namespace A113 { namespace BRp {


A113_BR_FNC RESULT IO_port::basic_read_loop( const BUFFER& buf_ ) {
    BUFFER::n_t n = 0;
    do {
        RESULT res = this->read( { ( char* )buf_.ptr + n, buf_.n - n } );
        A113_ASSERT_OR( res > 0 ) return res;
        n += res;
    } while( n < buf_.n );
    return n;
}

A113_BR_FNC RESULT IO_port::basic_write_loop( const BUFFER& buf_ ) {
    BUFFER::n_t n = 0;
    do {
        RESULT res = this->write( { ( char* )buf_.ptr + n, buf_.n - n } );
        A113_ASSERT_OR( res > 0 ) return res;
        n += res;
    } while( n < buf_.n );
    return n;
}


} };
