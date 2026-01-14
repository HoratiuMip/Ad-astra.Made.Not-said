/**
 * @file: BRp/IO_port.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/brp/IO_port.hpp>

namespace a113 { namespace io {


// A113_IMPL_FNC status_t Port::basic_read_loop( const port_RW_desc_t& desc_ ) {
//     MDsc::n_t n = 0;
//     do {
//         status_t status = this->read( { ( char* )mdsc_.ptr + n, mdsc_.n - n } );
//         A113_ASSERT_OR( status > 0 ) return status;
//         n += status;
//     } while( n < mdsc_.n );
//     return n;
// }

// A113_IMPL_FNC status_t Port::basic_write_loop( const port_RW_desc_t& desc_ ) {
//     MDsc::n_t n = 0;
//     do {
//         status_t status = this->write( { ( char* )mdsc_.ptr + n, mdsc_.n - n } );
//         A113_ASSERT_OR( status > 0 ) return status;
//         n += status;
//     } while( n < mdsc_.n );
//     return n;
// }


} };
